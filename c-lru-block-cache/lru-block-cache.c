#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* ===========================================================================
 *  UNRELATED TO CACHE: Wrapper for Block and Device and Data Types
 */
#define RFALSE      0
#define RTRUE       1

typedef unsigned long int xuint32_t;
typedef unsigned short int xuint16_t;
typedef unsigned char xuint8_t;
typedef unsigned char xbool_t;
typedef xuint32_t xxx_blkptr_t;

typedef struct {
    /* PUT YOUR BLOCK DATA HERE */
    xxx_blkptr_t index;
} xxx_block_t;

typedef struct {
    /* PUT YOUR DEVICE INFO HERE */
    xxx_blkptr_t block_size;
} xxx_device_t;

static void xxx_block_initialize (xxx_block_t *block, xxx_blkptr_t size)
{
    /* TODO: Alloc your block info here */
}

static void xxx_block_write (xxx_device_t *device, 
                             xxx_block_t *block)
{
    /* TODO: Write Specified Block to Device */
}

static void xxx_block_read (xxx_device_t *device, 
                            xxx_block_t *block)
{
    /* TODO: Read Specified Block from Device */
}

/* ===========================================================================
 *  Cache Data Structure and Hash Macro
 */
#define BUFHASH(cache, blocknr)     ((blocknr) % (cache)->buf_hash_size)

struct blkcache_buf {
    struct blkcache_buf *  next;    /* Next Queue Item */
    struct blkcache_buf *  prev;    /* Prev Queue Item */
    struct blkcache_buf *  hash;    /* Next Item with the same hash */

    xuint16_t              count;   /* Retain count */
    xxx_block_t            block;   /* Cached Block */
};

typedef struct {
    struct blkcache_buf ** buf_hash;        /* Bufs Hashtable */
    xuint16_t              buf_hash_size;   /* Bufs Hashtable Size */
    xuint16_t              buf_used;        /* Bufs in use */

    struct blkcache_buf *  head;            /* Head of the Bufs Queue */
    struct blkcache_buf *  tail;            /* Tail of the Bufs Queue */

    xxx_device_t *         device;          /* Block Device used for I/O */
} xxx_blkcache_t;


/* ===========================================================================
 *  PRIVATE Cache Functions
 */
static struct blkcache_buf *__blkcache_buf_alloc (xxx_blkcache_t *cache,
                                                  struct blkcache_buf *buf,
                                                  xxx_blkptr_t blocknr)
{
    if (buf == NULL) {
        if ((buf = malloc(sizeof(struct blkcache_buf))) == NULL)
            return(NULL);

        xxx_block_initialize(&(buf->block), cache->device->block_size);
    }

    buf->count = 0;
    buf->block.index = blocknr;
    buf->next = NULL;
    buf->prev = NULL;
    buf->hash = NULL;

    return(buf);
}

static void __blkcache_buf_free (struct blkcache_buf *buf) {
    free(buf);
}

static struct blkcache_buf *__blkcache_remove_lfu (xxx_blkcache_t *cache) {
    register struct blkcache_buf *p;
    struct blkcache_buf *buf;
    xuint16_t hash_index;

    /* Replace Head */
    buf = cache->head;
    cache->head = buf->next;
    cache->head->prev = NULL;

    /* Remove From Hash */
    hash_index = BUFHASH(cache, buf->block.index);
    if ((p = cache->buf_hash[hash_index]) == buf) {
        p = buf->hash;
        cache->buf_hash[hash_index] = p;
    } else {
        for (; p != NULL; p = p->hash) {
            if (p->hash == buf) {
                p->hash = buf->hash;
                break;
            }
        }
    }

    return(buf);
}

/* Shift to the right specified cache buf, delay block deletion */
static void __blkcache_buf_shift (xxx_blkcache_t *cache, 
                                  struct blkcache_buf *hbuf)
{
    register struct blkcache_buf *p;

    if (cache->tail == hbuf || hbuf->next == NULL)
        return;

    if (cache->head == hbuf)
        cache->head = hbuf->next;

    if ((p = hbuf->prev) != NULL)
        p->next = hbuf->next;

    if ((p = hbuf->next) != NULL) {
        p->prev = hbuf->prev;
        hbuf->next = p->next;
        hbuf->prev = p;
        p->next = hbuf;

        if ((p = hbuf->next) != NULL)
            p->prev = hbuf;
    }

    if (cache->tail == hbuf->prev)
        cache->tail = hbuf;
}

static struct blkcache_buf *__blkcache_find (xxx_blkcache_t *cache,
                                             xxx_blkptr_t blocknr,
                                             xuint16_t hash_index)
{
    register struct blkcache_buf *p;

    for (p = cache->buf_hash[hash_index]; p != NULL; p = p->hash) {
        if (p->block.index == blocknr)
            return(p);
    }

    return(NULL);
}

static void __blkcache_readwrite (xxx_blkcache_t *cache,
                                  struct blkcache_buf *buf,
                                  xbool_t is_writing)
{
    if (is_writing)
        xxx_block_write(cache->device, &(buf->block));
    else
        xxx_block_read(cache->device, &(buf->block));
}

/* ===========================================================================
 *  PUBLIC Cache Functions
 */
xbool_t xxx_blkcache_initialize  (xxx_blkcache_t *cache,
                                      xxx_device_t *device,
                                      xuint16_t max_size)
{
    cache->device = device;
    cache->buf_hash_size = max_size;

    cache->buf_hash = malloc(max_size * sizeof(struct blkcache_buf *));
    if (cache->buf_hash == NULL) return(RFALSE);

    memset(cache->buf_hash, 0, max_size * sizeof(struct blkcache_buf *));

    cache->head = NULL;
    cache->tail = NULL;
    cache->buf_used = 0;

    return(RTRUE);
}

void xxx_blkcache_flushall (xxx_blkcache_t *cache) {
    /* Flush Dirty Blocks */
}

void xxx_blkcache_uninitialize (xxx_blkcache_t *cache) {
    struct blkcache_buf *temp;
    struct blkcache_buf *p;

    /* Flush All Blocks */
    xxx_blkcache_flushall(cache);

    /* Free all Cache Items */
    for (p = cache->head; p != NULL; p = temp) {
        temp = p->next;
        __blkcache_buf_free(p);
    }

    free(cache->buf_hash);
}

xxx_block_t *xxx_blkcache_read (xxx_blkcache_t *cache,
                                xxx_blkptr_t blocknr)
{
    struct blkcache_buf *buf;
    xuint16_t hash_index;

    /* Scan the hash chain for block */
    hash_index = BUFHASH(cache, blocknr);
    if ((buf = __blkcache_find(cache, blocknr, hash_index)) != NULL) {
        buf->count++;

        /* Move Buf far from head */
        __blkcache_buf_shift(cache, buf);

        return(&(buf->block));
    }

    /* Cache is Full, Remove one Item */
    if ((cache->buf_used + 1) > cache->buf_hash_size) {
        /* All buffers are in use */
        if (cache->head->count > 0)
            return(NULL);

        /* Remove Least-Frequently Used */
        buf = __blkcache_remove_lfu(cache);
        cache->buf_used--;
    }

    /* Desidered block is not on available chain, Read It! */
    if ((buf = __blkcache_buf_alloc(cache, buf, blocknr)) == NULL)
        return(NULL);

    /* Add One Use, Block cannot be removed */
    buf->count++;

    /* Go get the requested block unless searching or prefetching. */
    __blkcache_readwrite(cache, buf, RFALSE);

    /* Update Cache Hash */
    cache->buf_used++;
    buf->hash = cache->buf_hash[hash_index];
    cache->buf_hash[hash_index] = buf;

    /* Update Cache Queue */
    if (cache->head == NULL) {
        cache->head = cache->tail = buf;
    } else {
        buf->prev = cache->tail;
        cache->tail->next = buf;
        cache->tail = buf;
    }

    return(&(buf->block));
}

void xxx_blkcache_release (xxx_blkcache_t *cache,
                           xxx_blkptr_t blocknr)
{
    struct blkcache_buf *buf;

    /* Find Block Buffer */
    if (!(buf = __blkcache_find(cache, blocknr, BUFHASH(cache, blocknr))))
        return;

    /* Remove One Use and check if Block is Still in Use */
    --(buf->count);
}

/* ===========================================================================
 *  TEST and DEBUG Functions
 */
void xxx_blkcache_debug (xxx_blkcache_t *cache) {
    struct blkcache_buf *p;
    xuint32_t n1, n2;
    xuint32_t sum = 0;

    for (n1 = 0, p = cache->head; p != NULL; p = p->next, ++n1) {
        printf("(%2lu) ", p->block.index);
        sum += p->block.index;
    }
    printf("\n");


    for (n2 = 0, p = cache->tail; p != NULL; p = p->prev, ++n2) {
        printf("(%2lu) ", p->block.index);
        sum -= p->block.index;
    }
    printf("\n\n");

    if (n1 != n2 || sum != 0) {
        printf("ABORTED\n");
        exit(1);
    }
}


xuint32_t __random (xuint32_t min, xuint32_t max) {
    return(min + (xuint32_t)(max * ((double)rand() / RAND_MAX)));
}

int main (int argc, char **argv) {
    xxx_blkcache_t cache;
    xxx_device_t device;
    xxx_block_t *block;
    xuint32_t k;
    xuint32_t i;
    
    device.block_size = 4096;
    xxx_blkcache_initialize(&cache, &device, 16 * 1024);
    
    srand(time(NULL));
    for (i = 0; i < 256 * 1024; ++i) {
        k = __random(0, 128 * 1024);
        block = xxx_blkcache_read(&cache, k);
        xxx_blkcache_release(&cache, k);
    }
    //xxx_blkcache_debug(&cache);

    xxx_blkcache_uninitialize(&cache);

    return(0);
}


