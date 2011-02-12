#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

typedef struct cgroup_ctrl {
    int efd;    /* event fd */
    int fd;     /* file fd */
} cgroup_ctrl_t;

struct list {
    struct list *next;
    uint8_t blob[1];
};

/*
 * Utity method that allows to write to a specified "cgroup/file"
 * the string ctrl with size ctrl_size.
 */
static int __cgroup_write (const char *cgroup,
                           const char *file,
                           const char *ctrl,
                           size_t ctrl_size)
{
    char path[PATH_MAX];
    ssize_t wr;
    int fd;

    snprintf(path, PATH_MAX, "%s/%s", cgroup, file);
    if ((fd = open(path, O_WRONLY)) < 0) {
        fprintf(stderr, "%s\n", path);
        perror("open()");
        return(-1);
    }

    fprintf(stderr, "WRITE %s %s %d\n", path, ctrl, ctrl_size);
    while (ctrl_size > 0) {
        if ((wr = write(fd, ctrl, ctrl_size)) < 0) {
            perror("write");
            close(fd);
            return(-2);
        }

        ctrl_size -= wr;
        ctrl += wr;
    }

    close(fd);

    return(0);
}

/*
 * Write to "cgroup/task" file specified pid.
 * (Aka. Move proces with specified pid to specified group.
 */
static int __cgroup_task (const char *cgroup,
                          int pid)
{
    char buffer[16];
    int n;
    n = snprintf(buffer, sizeof(buffer), "%d", pid);
    return(__cgroup_write(cgroup, "tasks", buffer, n));
}

/*
 * Write to "cgroup/cgroup.event_control" the specified ctrl string.
 */
static int __cgroup_event_control (const char *cgroup,
                                   const char *ctrl,
                                   size_t ctrl_size)
{
    return(__cgroup_write(cgroup, "cgroup.event_control", ctrl, ctrl_size));
}

/*
 * Initialize cgroup control data structure, opening "cgroup/file".
 */
int __cgroup_ctrl_init (cgroup_ctrl_t *cgroup_ctrl,
                        const char *cgroup,
                        const char *file)
{
    char path[PATH_MAX];

    snprintf(path, PATH_MAX, "%s/%s", cgroup, file);
    if ((cgroup_ctrl->fd = open(path, O_RDONLY)) < 0) {
        fprintf(stderr, "%s\n", path);
        perror("open()");
        return(-1);
    }

    if ((cgroup_ctrl->efd = eventfd(0, EFD_NONBLOCK)) < 0) {
        perror("eventfd()");
        close(cgroup_ctrl->fd);
        return(-2);
    }

    fprintf(stderr, "READ %d %s\n", cgroup_ctrl->fd, path);
    return(0);
}

/*
 * Close cgroup control data structure.
 */
int cgroup_ctrl_close (cgroup_ctrl_t *cgroup_ctrl) {
    close(cgroup_ctrl->efd);
    close(cgroup_ctrl->fd);
    return(0);
}

/*
 * Initialize a cgroup control to listen on memory usage
 * with specified threshold.
 */
int cgroup_memory_threshold (cgroup_ctrl_t *cgroup_ctrl,
                             const char *cgroup,
                             uint64_t threshold)
{
    char buffer[64];
    int n;

    if (__cgroup_ctrl_init(cgroup_ctrl, cgroup, "memory.usage_in_bytes") < 0)
        return(-1);

    n = snprintf(buffer, sizeof(buffer), "%d %d %"PRIu64"",
                 cgroup_ctrl->efd, cgroup_ctrl->fd, threshold);
    if (__cgroup_event_control(cgroup, buffer, n) < 0) {
        cgroup_ctrl_close(cgroup_ctrl);
        return(-2);
    }

    return(0);
}

/*
 * Helper method to add cgroup control to epoll.
 */
static int __epoll_add (int epfd, cgroup_ctrl_t *cgroup_ctrl) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = cgroup_ctrl->efd;
    return(epoll_ctl(epfd, EPOLL_CTL_ADD, cgroup_ctrl->efd, &ev));
}

#define KB(x)                       ((x) * 1024)
#define MB(x)                       (KB((x) * 1024))
#define BLOB_SIZE                   KB(512)

static struct list *__alloc_next (struct list *head) {
    struct list *node;

    node = malloc(sizeof(struct list) +  BLOB_SIZE);
    node->next = head;
    memset(node->blob, 5, BLOB_SIZE);
    return(node);
}

static struct list *__free_mem (struct list *head, unsigned int nodes) {
    unsigned int i;
    struct list *t;

    if (!nodes) {
        while (head != NULL) {
            t = head->next;
            free(t);
            head = t;
        }
    } else {
        for (i = 0; i < nodes; ++i) {
            t = head->next;

            free(head);
            if ((head = t) == NULL)
                break;
        }
    }

    return(head);
}

static void __play_with_mem (struct list *head) {
    unsigned int i;
    struct list *p;

    for (p = head; p != NULL; p = p->next) {
        for (i = 0; i < BLOB_SIZE; ++i)
            p->blob[i] = (p->blob[i] + 64) >> 1;
    }
}

int main (int argc, char **argv) {
    struct epoll_event events[2];
    cgroup_ctrl_t cgroup_ctrl[2];
    struct list *mem = NULL;
    unsigned int tloop = 2;
    int nfds, i;
    uint64_t r;
    int epfd;

    if (argc < 2) {
        printf("Usage: cgroup-mem-threshold <cgroup>\n");
        return(1);
    }

    /* Move this process to specified cgroup */
    __cgroup_task(argv[1], getpid());

    /* Initialize epoll */
    if ((epfd = epoll_create(2)) < 0) {
        perror("epoll_create()");
        return(-1);
    }

    /* Setup a mem threshold notifier at 6M */
    cgroup_memory_threshold(&(cgroup_ctrl[0]), argv[1], MB(6));
    __epoll_add(epfd, &(cgroup_ctrl[0]));

    /* Setup a mem threshold notifier at 12M */
    cgroup_memory_threshold(&(cgroup_ctrl[1]), argv[1], MB(12));
    __epoll_add(epfd, &(cgroup_ctrl[1]));

    while (1) {
        if ((nfds = epoll_wait(epfd, events, 2, 1000)) < 0) {
            perror("epoll_wait()");
            break;
        }

        /* Check for memory notification */
        for (i = 0; i < nfds; ++i) {
            printf("Event %d %d\n", i, events[i].data.fd);
            if (events[i].data.fd == cgroup_ctrl[0].efd) {
                read(cgroup_ctrl[0].efd, &r, sizeof(uint64_t));
                printf("Mem Threshold 6M Notification %"PRIu64"\n", r);
            }  else if (events[i].data.fd == cgroup_ctrl[1].efd) {
                read(cgroup_ctrl[1].efd, &r, sizeof(uint64_t));
                printf("Mem Threshold 12M Notification %"PRIu64"\n", r);

                if (--tloop > 0)
                    mem = __free_mem(mem, tloop);
            }
        }

        /* Allocate some momory */
        mem = __alloc_next(mem);
        __play_with_mem(mem);
    }

    __free_mem(mem, 0);
    close(epfd);
    cgroup_ctrl_close(&(cgroup_ctrl[0]));
    cgroup_ctrl_close(&(cgroup_ctrl[1]));

    return(0);
}

