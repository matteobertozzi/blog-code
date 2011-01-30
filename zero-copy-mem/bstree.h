#ifndef _BSTREE_H_
#define _BSTREE_H_

#define BSTREE_MAX_HEIGHT           (32)

typedef int (*compare_t) (void *, const void *, const void *);
typedef void (*free_t) (void *, void *);

typedef struct bstree_plug bstree_plug_t;
typedef struct bstnode bstnode_t;

typedef enum bstree_type {
    BSTREE_AVL,
    BSTREE_RED_BLACK,
} bstree_type_t;

typedef struct bstree_info {
    bstree_plug_t *plug;
    compare_t key_compare;
    free_t    data_free;
} bstree_info_t;

typedef struct bstree {
    bstree_info_t *info;
    bstnode_t *    root;
    void *         user_data;
} bstree_t;

typedef struct bstree_trav {
    const bstnode_t *stack[BSTREE_MAX_HEIGHT];
    const bstnode_t *current;
    const bstree_t *tree;
    unsigned int height;
} bstree_trav_t;

bstree_t *  bstree_alloc                (bstree_t *tree,
                                         bstree_type_t type,
                                         bstree_info_t *info);
void        bstree_free                 (bstree_t *tree);

void        bstree_clear                (bstree_t *tree);
int         bstree_insert               (bstree_t *tree,
                                         void *data);
int         bstree_remove               (bstree_t *tree,
                                         const void *key);
int         bstree_remove_range         (bstree_t *tree,
                                         const void *min_key,
                                         const void *max_key);
void *      bstree_lookup               (const bstree_t *tree,
                                         const void *key);
void *      bstree_lookup_min           (const bstree_t *tree);
void *      bstree_lookup_max           (const bstree_t *tree);
void *      bstree_lookup_lower         (const bstree_t *tree,
                                         const void *key,
                                         int equal);
void *      bstree_lookup_higher        (const bstree_t *tree,
                                         const void *key,
                                         int equal);
void *      bstree_lookup_custom        (const bstree_t *tree,
                                         const void *key,
                                         compare_t key_compare);
void        bstree_trav_init            (bstree_trav_t *trav,
                                         const bstree_t *tree);
void *      bstree_trav_current         (bstree_trav_t *trav);
void *      bstree_trav_next            (bstree_trav_t *trav);
void *      bstree_trav_prev            (bstree_trav_t *trav);
void *      bstree_trav_lookup          (bstree_trav_t *trav,
                                         const void *key);
void *      bstree_trav_lookup_min      (bstree_trav_t *trav);
void *      bstree_trav_lookup_max      (bstree_trav_t *trav);
void *      bstree_trav_lookup_lower    (bstree_trav_t *trav,
                                         const void *key,
                                         int equal);
void *      bstree_trav_lookup_higher   (bstree_trav_t *trav,
                                         const void *key,
                                         int equal);
void *      bstree_trav_lookup_custom   (bstree_trav_t *trav,
                                         const void *key,
                                         compare_t key_compare);

#endif /* _BSTREE_H_ */

