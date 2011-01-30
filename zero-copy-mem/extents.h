#ifndef _EXTENTS_H_
#define _EXTENTS_H_

#include <stdint.h>
#include "bstree.h"

#define EXTENT_TREE(x)                    ((extent_tree_t *)(x))

typedef int (*foreach_t) (void *, void *);

typedef struct blob {
    unsigned int  refs;
    unsigned int  size;
    unsigned char data[1];
} blob_t;

typedef struct segment {
    blob_t *blob;
    unsigned int offset;
    unsigned int length;
} segment_t;

typedef struct extent_tree {
    bstree_info_t info;
    bstree_t tree;
    uint64_t length;
} extent_tree_t;

segment_t * segment_alloc       (unsigned int size);
void        segment_free        (segment_t *segment);
segment_t * segment_copy        (segment_t *segment,
                                 unsigned int offset,
                                 unsigned int length);
int         segment_write       (segment_t *segment,
                                 unsigned int offset,
                                 const void *blob,
                                 unsigned int length);


int     extent_tree_open        (extent_tree_t *tree);
void    extent_tree_close       (extent_tree_t *tree);

int     extent_tree_append      (extent_tree_t *tree,
                                 segment_t *segment);
int     extent_tree_insert      (extent_tree_t *tree,
                                 uint64_t offset,
                                 segment_t *segment);
int     extent_tree_remove      (extent_tree_t *tree,
                                 uint64_t offset,
                                 uint64_t length);

void    extent_tree_foreach     (extent_tree_t *tree,
                                 foreach_t func,
                                 void *user_data);
void    extent_tree_traverse    (extent_tree_t *tree,
                                 uint64_t offset,
                                 uint64_t length,
                                 foreach_t func,
                                 void *user_data);

#endif /* !_EXTENTS_H_ */

