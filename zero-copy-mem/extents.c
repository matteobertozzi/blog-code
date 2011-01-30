#include <string.h>
#include <stdlib.h>
#include "extents.h"

#define EXTENT(x)                         ((extent_t *)(x))
#define SEGMENT(x)                        ((segment_t *)(x))

#define __extent_lookup(tree, extent)                                       \
    bstree_lookup_custom(&(tree->tree), extent, __lookup_compare)

#define __extent_lookup_left(tree, extent)                                  \
    bstree_lookup_custom(&(tree->tree), extent, __lookup_left_compare)

#define __extent_lookup_right(tree, extent)                                 \
    bstree_lookup_custom(&(tree->tree), extent, __lookup_right_compare)

typedef struct z_extent {
    uint64_t   offset;
    uint32_t   length;
    segment_t *segment;
} extent_t;

blob_t *blob_alloc (unsigned int size) {
    blob_t *blob;

    if ((blob = (blob_t *) malloc(sizeof(blob_t) + size)) != NULL) {
        blob->refs = 1;
        blob->size = size;
    }

    return(blob);
}

void blob_free (blob_t *blob) {
    if (--(blob->refs) == 0) {
        free(blob);
    }
}

segment_t *segment_alloc (unsigned int size) {
    segment_t *segment;
    blob_t *blob;

    if ((blob = blob_alloc(size)) == NULL)
        return(NULL);

    if ((segment = (segment_t *) malloc(sizeof(segment_t))) == NULL) {
        blob_free(blob);
        return(NULL);
    }

    segment->blob = blob;
    segment->offset = 0;
    segment->length = size;

    return(segment);
}

void segment_free (segment_t *segment) {
    blob_free(segment->blob);
    free(segment);
}

segment_t *segment_copy (segment_t *segment,
                         unsigned int offset,
                         unsigned int length)
{
    segment_t *dst;

    segment->blob->refs++;
    if ((dst = (segment_t *) malloc(sizeof(segment_t))) != NULL) {
        dst->blob = segment->blob;
        dst->offset = offset;
        dst->length = length;
    }

    return(dst);
}

int segment_write (segment_t *segment,
                   unsigned int offset,
                   const void *blob,
                   unsigned int length)
{
    /* TODO: This must be atomic */
    if (segment->blob->refs > 1) {
        blob_t *blob;

        if ((blob = blob_alloc(segment->length)) == NULL)
            return(-1);

        memcpy(blob->data, segment->blob->data + segment->offset, segment->length);
        segment->offset = 0;
        segment->blob->refs--;
    }

    memcpy(segment->blob->data + segment->offset + offset, blob, length);

    return(0);
}

static extent_t *__extent_alloc (extent_tree_t *tree,
                                 uint64_t offset,
                                 uint32_t length,
                                 segment_t *segment)
{
    extent_t *extent;

    if ((extent = (extent_t *) malloc(sizeof(extent_t))) != NULL) {
        extent->segment = segment;
        extent->offset = offset;
        extent->length = length;
    }

    return(extent);
}

static void __extent_free (void *user_data,
                           void *ptr)
{
    extent_t *extent = EXTENT(ptr);
    segment_free(extent->segment);
    free(extent);
}

static int __key_compare (void *user_data,
                          const void *a,
                          const void *b)
{
    const extent_t *ea = (const extent_t *)a;
    const extent_t *eb = (const extent_t *)b;
    uint64_t ah = ea->offset + ea->length - 1;
    uint64_t bh = eb->offset + eb->length - 1;
    return((ah < bh) ? -1 : ((ah > bh) ? 1 : 0));
}

static int __lookup_left_compare (void *user_data,
                                  const void *a,
                                  const void *b)
{
    const extent_t *ea = (const extent_t *)a;
    const extent_t *eb = (const extent_t *)b;
    uint64_t ah = ea->offset + ea->length - 1;
    uint64_t bh = eb->offset + eb->length - 1;

    /* check overlap */
    if (ea->offset <= bh && eb->offset <= ah) {
        if (ea->offset >= eb->offset && ea->offset <= bh)
            return(0);
        return(-1);
    }

    return((ah < bh) ? -1 : ((ah > bh) ? 1 : 0));
}

static int __lookup_right_compare (void *user_data,
                                   const void *a,
                                   const void *b)
{
    const extent_t *ea = (const extent_t *)a;
    const extent_t *eb = (const extent_t *)b;
    uint64_t ah = ea->offset + ea->length - 1;
    uint64_t bh = eb->offset + eb->length - 1;

    /* check overlap */
    if (ea->offset <= bh && eb->offset <= ah) {
        if (ah >= eb->offset && ah <= bh)
            return(0);
        return(1);
    }

    return((ah < bh) ? -1 : ((ah > bh) ? 1 : 0));
}

static void __extents_right_shift (extent_tree_t *tree,
                                   extent_t *extent)
{
    bstree_trav_t trav;
    extent_t *e;

    /* TODO: Optimize */
    bstree_trav_init(&trav, &(tree->tree));
    do {
        e = EXTENT(bstree_trav_current(&trav));
        if (e->offset >= extent->offset)
            e->offset += extent->length;
    } while (bstree_trav_next(&trav));
}

static void __extents_left_shift (extent_tree_t *tree,
                                  extent_t *extent)
{
    bstree_trav_t trav;
    extent_t *e;

    /* TODO: Optimize */
    bstree_trav_init(&trav, &(tree->tree));
    do {
        e = EXTENT(bstree_trav_current(&trav));
        if (e->offset > extent->offset)
            e->offset -= extent->length;
    } while (bstree_trav_next(&trav));
}

static int __extent_trim (extent_tree_t *tree,
                          extent_t *extent,
                          uint32_t trim)
{
    extent->length -= trim;
    extent->segment->length -= trim;
    return(0);
}

static int __extent_pre_trim (extent_tree_t *tree,
                              extent_t *extent,
                              uint32_t trim)
{
    extent->length -= trim;

    extent->segment->offset += trim;
    extent->segment->length -= trim;

    return(0);
}

static int __extent_split (extent_tree_t *tree,
                           extent_t *left,
                           uint64_t offset)
{
    uint32_t right_size;
    uint64_t right_off;
    segment_t *segment;
    extent_t *right;

    /* Calculate right extent Length */
    right_size = left->length - (offset - left->offset);
    right_off = left->segment->offset + (offset - left->offset);

    if ((segment = segment_copy(left->segment, right_off, right_size)) == NULL)
        return(-1);

    /* And allocate right extent */
    if ((right = __extent_alloc(tree, offset, right_size, segment)) == NULL)
        return(-2);

    /* Adjust Left Extent Length */
    left->length -= right->length;
    left->segment->length -= right->length;

    return(bstree_insert(&(tree->tree), right));
}

int extent_tree_open (extent_tree_t *tree)
{
    tree->info.key_compare = __key_compare;
    tree->info.data_free = NULL;
    bstree_alloc(&(tree->tree), BSTREE_AVL, &(tree->info));
    tree->length = 0;
    return(0);
}

void extent_tree_close (extent_tree_t *tree) {
    bstree_free(&(tree->tree));
}

int extent_tree_append (extent_tree_t *tree,
                        segment_t *segment)
{
    return(extent_tree_insert(tree, tree->length, segment));
}

int extent_tree_insert (extent_tree_t *tree,
                        uint64_t offset,
                        segment_t *segment)
{
    extent_t *extent;
    extent_t *left;

    if (!(extent = __extent_alloc(tree, offset, segment->length, segment)))
        return(2);

    if ((left = __extent_lookup_left(tree, extent)) != NULL) {
        if (left->offset < extent->offset) {
            /* Split extent */
            if (__extent_split(tree, left, extent->offset))
                return(1);
        }

        /* Shift Everything to the right */
        __extents_right_shift(tree, extent);
    }

    if (bstree_insert(&(tree->tree), extent)) {
        __extent_free(tree, extent);
        return(2);
    }

    tree->length += extent->length;
    return(0);
}


int extent_tree_remove (extent_tree_t *tree,
                        uint64_t offset,
                        uint64_t length)
{
    extent_t extent;
    extent_t *right;
    extent_t *left;
    uint64_t rh;
    uint64_t h;

    extent.offset = offset;
    extent.length = length;
    h = offset + length - 1;

    if ((left = __extent_lookup_left(tree, &extent)) == NULL)
        return(1);

    if ((right = __extent_lookup_right(tree, &extent)) == NULL)
        right = left;

    rh = right->offset + right->length - 1;

    if (left == right) {
        if (offset == left->offset && h == rh) {
            bstree_remove(&(tree->tree), left);
        } else if (offset == left->offset) {
            __extent_pre_trim(tree, left, length);
        } else if (h == rh) {
            __extent_trim(tree, left, length);
        } else {
            __extent_split(tree, left, h);
            __extent_trim(tree, left, length);
        }
    } else {
        bstree_remove_range(&(tree->tree), left, right);

        if (offset == left->offset && h == rh) {
            bstree_remove(&(tree->tree), left);
            bstree_remove(&(tree->tree), right);
        } else if (offset == left->offset) {
            bstree_remove(&(tree->tree), left);
            right->offset -= length - (right->length - (rh - h));
            __extent_pre_trim(tree, right, right->length - (rh - h));
        } else if (h == rh) {
            bstree_remove(&(tree->tree), right);
            __extent_trim(tree, left, (left->offset + left->length) - offset);
        } else {
            right->offset -= length - (right->length - (rh - h));
            __extent_trim(tree, left, (left->offset + left->length) - offset);
            __extent_pre_trim(tree, right, right->length - (rh - h));
        }
    }

    __extents_left_shift(tree, &extent);
    tree->length -= extent.length;

    return(0);
}

void extent_tree_foreach (extent_tree_t *tree,
                          foreach_t func,
                          void *user_data)
{
    bstree_trav_t trav;
    extent_t *e;

    if (tree->tree.root == NULL)
        return;

    bstree_trav_init(&trav, &(tree->tree));
    do {
        e = EXTENT(bstree_trav_current(&trav));
        if (!func(user_data, e->segment))
            return;
    } while (bstree_trav_next(&trav));
}

void extent_tree_traverse (extent_tree_t *tree,
                           uint64_t offset,
                           uint64_t length,
                           foreach_t func,
                           void *user_data)
{
    bstree_trav_t trav;
    extent_t extent;
    extent_t *right;
    extent_t *left;
    extent_t *e;

    extent.offset = offset;
    extent.length = length;

    if (tree->tree.root == NULL)
        return;

    if ((left = __extent_lookup_left(tree, &extent)) == NULL)
        return;

    if ((right = __extent_lookup_right(tree, &extent)) == NULL)
        right = left;

    bstree_trav_init(&trav, &(tree->tree));
    bstree_trav_lookup(&trav, left);
    do {
        e = EXTENT(bstree_trav_current(&trav));
        if (!func(user_data, e->segment))
            return;

        if (e == right)
            break;
    } while (bstree_trav_next(&trav));
}

#include <string.h>
#include <stdio.h>

static int __print_func (void *user_data, void *s) {
    segment_t *segment = (segment_t *)s;
    char buffer[1024];

    memcpy(buffer, segment->blob->data + segment->offset, segment->length);
    buffer[segment->length] = '\0';

    printf("%2u:%2u - %s\n", segment->offset, segment->length, buffer);
    return(1);
}

int main (int argc, char **argv) {
    segment_t *segment;
    extent_tree_t tree;

    extent_tree_open(&tree);

    segment = segment_alloc(12);
    memcpy(segment->blob->data, "Hello  World", 12);
    extent_tree_append(&tree, segment);
    extent_tree_foreach(&tree, __print_func, NULL);

    segment = segment_alloc(18);
    memcpy(segment->blob->data, ". This is a Tesht!", 18);
    extent_tree_append(&tree, segment);
    extent_tree_foreach(&tree, __print_func, NULL);

    segment = segment_alloc(4);
    memcpy(segment->blob->data, "Test", 4);
    extent_tree_insert(&tree, 6, segment);
    extent_tree_foreach(&tree, __print_func, NULL);

    extent_tree_remove(&tree, 24, 5);
    extent_tree_foreach(&tree, __print_func, NULL);

    extent_tree_close(&tree);

    return(0);
}

