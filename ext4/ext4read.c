#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#include "ext4.h"

#define SUPER_OFFSET 1024

typedef struct filesystem {
    struct ext4_super_block super;
    struct ext4_group_desc *groups;

    uint8_t *block;                     /* Block Buffer */
    int fd;                             /* Disk Descriptor */
} filesystem_t;

typedef struct directory {
    struct ext4_inode inode;
    uint64_t readed;
    uint32_t offset;
    uint32_t nblock;
} directory_t;

static int      directory_open   (filesystem_t *fs,
                                  directory_t *dir,
                                  uint32_t inode_n);
static int      directory_close  (filesystem_t *fs,
                                  directory_t *dir);
static int      directory_read   (filesystem_t *fs,
                                  directory_t *dir,
                                  struct ext4_dir_entry *entry);
static uint32_t directory_lookup (filesystem_t *fs,
                                  uint32_t dir_inode_n,
                                  const char *name,
                                  uint16_t name_len);

#define BLOCK_SIZE(fs)                  \
    (1024 << (fs)->super.s_log_block_size)

#define BLOCK_TO_OFFSET(fs, block)      \
    (block * BLOCK_SIZE(fs))

#define MAX_INDIRECT_BLOCK(fs)          \
    (EXT4_NDIR_BLOCKS + (BLOCK_SIZE(fs) / sizeof(uint32_t)))

#define ALIGN_DOWN(x, align)    ((x) & (-(align)))
#define ALIGN_UP(x, align)      (((x) + ((align) - 1) & (-(align))))

#define ALIGN_BLOCK(fs, x)              \
    ALIGN_UP(x, BLOCK_SIZE(fs))

static void __print_super (struct ext4_super_block *super) {
    printf("Ext4 Super Block\n");
    printf("==========================================\n");
    printf("Magic:                  %x\n", super->s_magic);
    printf("Block Size:             %u\n", 1024 << super->s_log_block_size);
    printf("Blocks Count:           %u\n", super->s_blocks_count_lo);
    printf("Reserved Blocks Count:  %u\n", super->s_r_blocks_count_lo);
    printf("Free Blocks Count:      %u\n", super->s_free_blocks_count_lo);
    printf("Free Inodes Count:      %u\n", super->s_free_inodes_count);
    printf("First Data Block:       %u\n", super->s_first_data_block);
    printf("Group Desc Size:        %u\n", super->s_desc_size);
    printf("N Block Groups:         %u\n", super->s_blocks_per_group);
    printf("Inodes per group:       %u\n", super->s_inodes_per_group);
    printf("Inode size:             %u\n", super->s_inode_size);
    printf("First Inode:            %u\n", super->s_first_ino);
    printf("\n");
}

static void __print_group_desc (struct ext4_group_desc *group) {
    printf("Ext4 Group Desc\n");
    printf("==========================================\n");
    printf("Blocks bitmap block: %u\n", group->bg_block_bitmap_lo);
    printf("Inodes bitmap block: %u\n", group->bg_inode_bitmap_lo);
    printf("Inodes table block: %u\n", group->bg_inode_table_lo);
    printf("Free blocks count: %u\n", group->bg_free_blocks_count_lo);
    printf("Free inodes count: %u\n", group->bg_free_inodes_count_lo);
    printf("Directories count: %u\n", group->bg_used_dirs_count_lo);
    printf("crc16(sb_uuid+group+desc): %u\n", group->bg_checksum);
    printf("\n");
}

static void __print_inode (struct ext4_inode *inode) {
    unsigned int i;

    printf("Ext4 I-Node\n");
    printf("==========================================\n");
    printf("File mode:    %o\n", inode->i_mode);
    printf("UID low:      %u\n", inode->i_uid);
    printf("Size Lo:      %u\n", inode->i_size_lo);
    printf("Size Hi:      %u\n", inode->i_size_high);
    printf("GID low:      %u\n", inode->i_gid);
    printf("Link count:   %u\n", inode->i_links_count);
    printf("Blocks count: %u\n", inode->i_blocks_lo);
    printf("File Flags:   %u\n", inode->i_flags);

    for (i = 0; i < EXT4_N_BLOCKS; ++i)
        printf("i_block[%2u]: %u\n", i, inode->i_block[i]);

    printf("\n");
}

static int _inode_by_number (filesystem_t *fs,
                             struct ext4_inode *inode,
                             uint32_t inode_n)
{
    uint32_t group;
    uint64_t block;
    off_t offset;

    /* I-Node starts from 1 */
    inode_n--;

    /* Lookup I-Node Offset */
    group = inode_n / fs->super.s_inodes_per_group;
    block = BLOCK_TO_OFFSET(fs, fs->groups[group].bg_inode_table_lo);
    offset = (inode_n % fs->super.s_inodes_per_group) * fs->super.s_inode_size;
    __print_group_desc(&(fs->groups[group]));

    /* Read I-Node */
    if (pread(fs->fd, inode, sizeof(struct ext4_inode), block + offset) < 0)
        return(-1);

    return(0);
}

static int _inode_by_path (filesystem_t *fs,
                           struct ext4_inode *inode,
                           const char *path)
{
    char name[EXT4_NAME_LEN + 1];
    const char *p1, *p2;
    unsigned int len;
    uint32_t inode_n;

    inode_n = EXT4_ROOT_INO;

    p1 = path;
    while ((p1 = strchr(p1, '/')) != NULL) {
        p2 = strchr(++p1, '/');

        if ((len = (p2 != NULL) ? (p2 - p1) : strlen(p1)) > EXT4_NAME_LEN)
            len = EXT4_NAME_LEN;
        memcpy(name, p1, len);
        name[len] = '\0';
        printf("%s\n", name);

        if ((inode_n = directory_lookup(fs, inode_n, name, len)) == EXT4_BAD_INO)
            return(-1);

        if ((p1 = p2) == NULL)
            break;
    }

    if (_inode_by_number(fs, inode, inode_n) < 0)
        return(-1);

    return(0);
}

static uint32_t _inode_get_data_blocks (filesystem_t *fs,
                                        struct ext4_inode *inode,
                                        uint32_t block,
                                        uint32_t *extent)
{
    if (extent != NULL)
        *extent = 1;

    if (inode->i_flags & EXT4_EXTENTS_FL) {
        struct ext4_extent_header *header;

        header = (struct ext4_extent_header *)(inode->i_block);
        printf("Read Extents %u %u %u\n", header->eh_depth, header->eh_entries, header->eh_max);

        if (header->eh_depth == 0) {
            struct ext4_extent *extent;
            unsigned int i;

            extent = (struct ext4_extent *)(header + 1);
            for (i = 0; i < header->eh_entries; ++i) {
                printf("%6u %6u %6u %6u\n", extent->ee_block, extent->ee_len, extent->ee_start_hi, extent->ee_start_lo);
                extent++;
            }
        } else {
            struct ext4_extent_idx *eidx;

            eidx = (struct ext4_extent_idx *)(header + 1);
            /* Index of extent blocks */
        }
    } else {
        if (block < EXT4_NDIR_BLOCKS) {
            /* Direct block (I-Node pointers list) */
            return(inode->i_block[block]);
        } else if (block < MAX_INDIRECT_BLOCK(fs)) {
            /* Indirect Block */
            printf("Read Indirect Block\n");
        } else {
            /* Double-Indirect Block */
            printf("Read Double-Indirect Block\n");
        }
    }

    return(0);
}

static int _group_table_load (filesystem_t *fs) {
    uint32_t i, groups_count;
    uint32_t group_size;
    off_t offset;

    groups_count = fs->super.s_blocks_count_lo / fs->super.s_blocks_per_group;
    if (groups_count == 0)
        groups_count = 1;

    if (!(fs->groups = malloc(sizeof(struct ext4_group_desc) * groups_count))) {
        close(fs->fd);
        return(-1);
    }

    if (fs->super.s_desc_size != 0)
        group_size = sizeof(struct ext4_group_desc);
    else
        group_size = 0x20;

    /* Read all groups */
    offset = ALIGN_BLOCK(fs, SUPER_OFFSET + sizeof(struct ext4_super_block));
    for (i = 0; i < groups_count; ++i) {
        if (pread(fs->fd, &(fs->groups[i]), group_size, offset) < 0)
            return(-1);

        offset += group_size;
    }

    return(0);
}

static int filesystem_open (filesystem_t *fs, const char *path) {
    if ((fs->fd = open(path, O_RDONLY)) < 0) {
        perror("open()");
        return(-1);
    }

    pread(fs->fd, &(fs->super), sizeof(struct ext4_super_block), SUPER_OFFSET);
    __print_super(&(fs->super));
    /* fs->block = (uint8_t *) malloc(); */

    _group_table_load(fs);

    return(0);
}

static int filesystem_close (filesystem_t *fs) {
    close(fs->fd);
    free(fs->groups);
    //free(fs->block);
    return(0);
}

static void _read_some (filesystem_t *fs,
                        uint64_t block,
                        uint32_t size)
{
    char buffer[64 * 1024];
    unsigned int i;

    pread(fs->fd, buffer, size, BLOCK_TO_OFFSET(fs, block));
    buffer[size] = 0;

    for (i = 0; i < size; ++i)
        printf("%c", buffer[i]);
    printf("\n");
}

static int directory_open (filesystem_t *fs,
                           directory_t *dir,
                           uint32_t inode_n)
{
    if (_inode_by_number(fs, &(dir->inode), inode_n) < 0)
        return(-1);

    if (!S_ISDIR(dir->inode.i_mode))
        return(-2);

    dir->readed = 0;
    dir->offset = 0;
    dir->nblock = 0;

    return(0);
}

static int directory_close (filesystem_t *fs,
                            directory_t *dir)
{
    return(0);
}

static int directory_read (filesystem_t *fs,
                           directory_t *dir,
                           struct ext4_dir_entry *entry)
{
    uint64_t offset;
    uint32_t block;

    if (dir->readed >= dir->inode.i_size_lo)
        return(-1);

    block = _inode_get_data_blocks(fs, &(dir->inode), dir->nblock, NULL);
    offset = BLOCK_TO_OFFSET(fs, block) + dir->offset;

    if (pread(fs->fd, entry, sizeof(struct ext4_dir_entry), offset) < 0)
        return(-2);

    dir->readed += entry->rec_len;
    if ((dir->offset += entry->rec_len) >= BLOCK_SIZE(fs))
        dir->nblock++;

    return(0);
}

static uint32_t directory_lookup (filesystem_t *fs,
                                  uint32_t dir_inode_n,
                                  const char *name,
                                  uint16_t name_len)
{
    struct ext4_dir_entry entry;
    directory_t dir;

    if (directory_open(fs, &dir, dir_inode_n) < 0)
        return(EXT4_BAD_INO);

    while (!directory_read(fs, &dir, &entry)) {
        printf("%s %s\n", entry.name, name);
        if (entry.name_len != name_len)
            continue;

        if (!memcmp(entry.name, name, entry.name_len))
            return(entry.inode);
    }

    directory_close(fs, &dir);

    return(EXT4_BAD_INO);
}

int main (int argc, char **argv) {
    struct ext4_inode inode;
    filesystem_t fs;

    if (argc < 2) {
        fprintf(stderr, "ext4read <path>\n");
        return(1);
    }

    filesystem_open(&fs, argv[1]);

#if 0
    _inode_by_number(&fs, &inode, EXT4_ROOT_INO);
    __print_inode(&inode);
    _inode_get_data_blocks(&fs, &inode, 0, NULL);
    _read_dir(&fs, inode.i_block[0], 512);
#else
    if (_inode_by_path(&fs, &inode, "/file1.txt") < 0)
        printf("_inode_by_path(): error\n");
    __print_inode(&inode);
    _inode_get_data_blocks(&fs, &inode, 0, NULL);
#endif

    filesystem_close(&fs);

    return(0);
}

