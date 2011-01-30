#include <stdlib.h>

#include "bstree.h"

struct bstree_plug {
    int (*insert) (bstree_t *, void *);
    int (*remove) (bstree_t *, const void *);
};

struct bstnode {
    struct bstnode *child[2];
    void *data;
    unsigned int balance;
};

static bstnode_t *__bstnode_alloc (bstree_t *tree, void *data) {
    bstnode_t *node;

    if ((node = (bstnode_t *) malloc(sizeof(bstnode_t))) != NULL) {
        node->data = data;
        node->child[1] = NULL;
        node->child[0] = NULL;
        node->balance = 0;
    }

    return(node);
}

static void __bstnode_free (bstree_t *tree, bstnode_t *node) {
    free_t data_free;

    if ((data_free = tree->info->data_free) != NULL)
        data_free(tree->user_data, node->data);

    free(node);
}

static void *__trav_move (bstree_trav_t *trav, int dir) {
    const bstnode_t *p;
    int ndir = !dir;

    if ((p = trav->current) == NULL)
        return(NULL);

    if (p->child[ndir] != NULL) {
        trav->stack[trav->height++] = p;
        p = p->child[ndir];

        while (p->child[dir] != NULL) {
            trav->stack[trav->height++] = p;
            p = p->child[dir];
        }
    } else {
        const bstnode_t *tmp;

        do {
            if (trav->height == 0) {
                trav->current = NULL;
                return(NULL);
            }

            tmp = p;
            p = trav->stack[--(trav->height)];
        } while (p->child[ndir] == tmp);
    }

    trav->current = p;
    return(p->data);
}

static void *__trav_lookup (bstree_trav_t *trav, int right) {
    const bstnode_t *p;

    trav->height = 0;
    trav->current = NULL;
    for (p = trav->tree->root; p != NULL; p = p->child[0]) {
        if (p->child[0] == NULL) {
            trav->current = p;
            return(p->data);
        }

        trav->stack[trav->height++] = p;
    }

    trav->height = 0;
    return(NULL);
}

static int __redblack_insert (bstree_t *tree, void *data) {
    return(-1);
}

static int __redblack_remove (bstree_t *tree, const void *key) {
    return(-1);
}

static int __avl_insert (bstree_t *tree, void *data) {
  unsigned char da[BSTREE_MAX_HEIGHT];
    compare_t key_compare;
    free_t data_free;
    bstnode_t *y, *z;
    bstnode_t *p, *q;
    bstnode_t *n;
    bstnode_t *w;
    int k = 0;
    int dir;
    int cmp;

    key_compare = tree->info->key_compare;
    data_free = tree->info->data_free;

    z = (bstnode_t *) &tree->root;
    y = tree->root;
    dir = 0;
    for (q = z, p = y; p != NULL; q = p, p = p->child[dir]) {
        if (!(cmp = key_compare(tree->user_data, data, p->data))) {
            if (p->data != data && data_free != NULL)
                data_free(tree->user_data, p->data);

            p->data = data;
            return(0);
        }

        if (p->balance != 0)
            z = q, y = p, k = 0;
        da[k++] = dir = cmp > 0;
    }

    n = q->child[dir] = __bstnode_alloc(tree, data);
    if (n == NULL)
        return(-1);

    n->data = data;
    n->child[0] = n->child[1] = NULL;
    n->balance = 0;
    if (y == NULL)
        return(0);

    for (p = y, k = 0; p != n; p = p->child[da[k]], k++) {
        if (da[k] == 0)
            p->balance--;
        else
            p->balance++;
    }

    if (y->balance == -2) {
        bstnode_t *x = y->child[0];
        if (x->balance == -1) {
            w = x;
            y->child[0] = x->child[1];
            x->child[1] = y;
            x->balance = y->balance = 0;
        } else {
            w = x->child[1];
            x->child[1] = w->child[0];
            w->child[0] = x;
            y->child[0] = w->child[1];
            w->child[1] = y;
            if (w->balance == -1)
                x->balance = 0, y->balance = +1;
            else if (w->balance == 0)
                x->balance = y->balance = 0;
            else /* |w->balance == +1| */
                x->balance = -1, y->balance = 0;
            w->balance = 0;
        }
    } else if (y->balance == +2) {
        bstnode_t *x = y->child[1];
        if (x->balance == +1) {
            w = x;
            y->child[1] = x->child[0];
            x->child[0] = y;
            x->balance = y->balance = 0;
        } else {
            w = x->child[0];
            x->child[0] = w->child[1];
            w->child[1] = x;
            y->child[1] = w->child[0];
            w->child[0] = y;
            if (w->balance == +1)
                x->balance = 0, y->balance = -1;
            else if (w->balance == 0)
                x->balance = y->balance = 0;
            else /* |w->balance == -1| */
                x->balance = +1, y->balance = 0;
            w->balance = 0;
        }
    } else {
        return(0);
    }
    z->child[y != z->child[0]] = w;

    return(0);
}

static int __avl_remove (bstree_t *tree, const void *key) {
    unsigned char da[BSTREE_MAX_HEIGHT];
    bstnode_t *pa[BSTREE_MAX_HEIGHT];
    compare_t key_compare;
    free_t data_free;
    bstnode_t *p;
    int cmp, dir;
    int k;

    key_compare = tree->info->key_compare;
    data_free = tree->info->data_free;

    k = 0;
    dir = 0;
    p = (bstnode_t *)&(tree->root);
    while (1) {
        pa[k] = p;
        da[k++] = dir;

        if ((p = p->child[dir]) == NULL)
            return(1);

        if (!(cmp = key_compare(tree->user_data, key, p->data)))
            break;

        dir = cmp > 0;
    }

    if (p->child[1] == NULL) {
        pa[k - 1]->child[da[k - 1]] = p->child[0];
    } else {
        bstnode_t *r = p->child[1];
        if (r->child[0] == NULL) {
            r->child[0] = p->child[0];
            r->balance = p->balance;
            pa[k - 1]->child[da[k - 1]] = r;
            da[k] = 1;
            pa[k++] = r;
        } else {
            bstnode_t *s;
            int j = k++;

            for (;;) {
                da[k] = 0;
                pa[k++] = r;
                s = r->child[0];
                if (s->child[0] == NULL)
                    break;

                r = s;
            }

            s->child[0] = p->child[0];
            r->child[0] = s->child[1];
            s->child[1] = p->child[1];
            s->balance = p->balance;

            pa[j - 1]->child[da[j - 1]] = s;
            da[j] = 1;
            pa[j] = s;
        }
    }

    __bstnode_free(tree, p);

    while (--k > 0) {
        bstnode_t *y = pa[k];

        if (da[k] == 0) {
            y->balance++;
            if (y->balance == +1)
                break;

            if (y->balance == +2) {
                bstnode_t *x = y->child[1];
                if (x->balance == -1) {
                    bstnode_t *w;
                    w = x->child[0];
                    x->child[0] = w->child[1];
                    w->child[1] = x;
                    y->child[1] = w->child[0];
                    w->child[0] = y;
                    if (w->balance == +1)
                        x->balance = 0, y->balance = -1;
                    else if (w->balance == 0)
                        x->balance = y->balance = 0;
                    else /* |w->balance == -1| */
                        x->balance = +1, y->balance = 0;
                    w->balance = 0;
                    pa[k - 1]->child[da[k - 1]] = w;
                } else {
                    y->child[1] = x->child[0];
                    x->child[0] = y;
                    pa[k - 1]->child[da[k - 1]] = x;
                    if (x->balance == 0) {
                        x->balance = -1;
                        y->balance = +1;
                        break;
                    } else {
                        x->balance = y->balance = 0;
                    }
                }
            }
        } else {
            y->balance--;
            if (y->balance == -1)
                break;

            if (y->balance == -2) {
                bstnode_t *x = y->child[0];
                if (x->balance == +1) {
                    bstnode_t *w;
                    w = x->child[1];
                    x->child[1] = w->child[0];
                    w->child[0] = x;
                    y->child[0] = w->child[1];
                    w->child[1] = y;
                    if (w->balance == -1)
                        x->balance = 0, y->balance = +1;
                    else if (w->balance == 0)
                        x->balance = y->balance = 0;
                    else /* |w->balance == +1| */
                        x->balance = -1, y->balance = 0;
                    w->balance = 0;
                    pa[k - 1]->child[da[k - 1]] = w;
                } else {
                    y->child[0] = x->child[1];
                    x->child[1] = y;
                    pa[k - 1]->child[da[k - 1]] = x;
                    if (x->balance == 0) {
                        x->balance = +1;
                        y->balance = -1;
                        break;
                    } else {
                        x->balance = y->balance = 0;
                    }
                }
            }
        }
    }

    return(0);
}

static bstree_plug_t __redblack_tree_plug = {
    .insert = __redblack_insert,
    .remove = __redblack_remove,
};

static bstree_plug_t __avl_tree_plug = {
    .insert = __avl_insert,
    .remove = __avl_remove,
};

bstree_t *bstree_alloc (bstree_t *tree,
                        bstree_type_t type,
                        bstree_info_t *info)
{
    tree->root = NULL;
    tree->info = info;

    switch (type) {
        case BSTREE_AVL:
            tree->info->plug = &__avl_tree_plug;
            break;
        case BSTREE_RED_BLACK:
            tree->info->plug = &__redblack_tree_plug;
            break;
    }

    return(tree);
}

void bstree_free (bstree_t *tree) {
    bstree_clear(tree);
}

void bstree_clear (bstree_t *tree) {
    bstnode_t *p, *q;

    for (p = tree->root; p != NULL; p = q) {
        if (p->child[0] == NULL) {
            q = p->child[1];
            __bstnode_free(tree, p);
        } else {
            q = p->child[0];
            p->child[0] = q->child[1];
            q->child[1] = p;
        }
    }

    tree->root = NULL;
}

int bstree_insert (bstree_t *tree, void *data) {
    return(tree->info->plug->insert(tree, data));
}

int bstree_remove (bstree_t *tree, const void *key) {
    return(tree->info->plug->remove(tree, key));
}

int bstree_remove_range (bstree_t *tree,
                         const void *min_key,
                         const void *max_key)
{
    bstree_trav_t traverser;
    compare_t key_compare;
    const void *key;

    bstree_trav_init(&traverser, tree);
    bstree_trav_lookup_higher(&traverser, min_key, 0);

    key_compare = tree->info->key_compare;
    while ((key = bstree_trav_current(&traverser)) != NULL) {
        if (key_compare(tree->user_data, max_key, key) <= 0)
            break;

        tree->info->plug->remove(tree, key);
        bstree_trav_lookup_higher(&traverser, min_key, 0);
    }

    return(0);
}

void *bstree_lookup (const bstree_t *tree, const void *key) {
    return(bstree_lookup_custom(tree, key, tree->info->key_compare));
}

void *bstree_lookup_min (const bstree_t *tree) {
    const bstnode_t *p;
    void *data = NULL;

    for (p = tree->root; p != NULL; p = p->child[0])
        data = p->data;

    return(data);
}

void *bstree_lookup_max (const bstree_t *tree) {
    const bstnode_t *p;
    void *data = NULL;

    for (p = tree->root; p != NULL; p = p->child[1])
        data = p->data;

    return(data);
}

void *bstree_lookup_lower (const bstree_t *tree,
                           const void *key,
                           int equal)
{
    bstree_trav_t trav;
    bstree_trav_init(&trav, tree);
    return(bstree_trav_lookup_lower(&trav, key, equal));
}

void *bstree_lookup_higher (const bstree_t *tree,
                            const void *key,
                            int equal)
{
    bstree_trav_t trav;
    bstree_trav_init(&trav, tree);
    return(bstree_trav_lookup_higher(&trav, key, equal));
}

void *bstree_lookup_custom (const bstree_t *tree,
                            const void *key,
                            compare_t key_compare)
{
    const bstnode_t *p;
    int cmp;

    p = tree->root;
    while (p != NULL) {
        if (!(cmp = key_compare(tree->user_data, key, p->data)))
            return(p->data);

        p = p->child[cmp > 0];
    }

    return(NULL);
}

void bstree_trav_init (bstree_trav_t *trav, const bstree_t *tree) {
    trav->tree = tree;
    trav->current = NULL;
    trav->height = 0;

    __trav_lookup(trav, 0);
}

void *bstree_trav_current (bstree_trav_t *trav) {
    if (trav->current == NULL)
        return(NULL);
    return(trav->current->data);
}

void *bstree_trav_next (bstree_trav_t *trav) {
    return(__trav_move(trav, 0));
}

void *bstree_trav_prev (bstree_trav_t *trav) {
    return(__trav_move(trav, 1));
}

void *bstree_trav_lookup (bstree_trav_t *trav, const void *key) {
    return(bstree_trav_lookup_custom(trav, key, trav->tree->info->key_compare));
}

void *bstree_trav_lookup_min (bstree_trav_t *trav) {
    return(__trav_lookup(trav, 0));
}

void *bstree_trav_lookup_max (bstree_trav_t *trav) {
    return(__trav_lookup(trav, 1));
}

void *bstree_trav_lookup_lower (bstree_trav_t *trav,
                                const void *key,
                                int equal)
{
    compare_t key_compare;
    const bstnode_t *p;
    void *user_data;
    int cmp;

    trav->height = 0;
    trav->current = NULL;

    p = trav->tree->root;
    user_data = trav->tree->user_data;
    key_compare = trav->tree->info->key_compare;

    while (p != NULL) {
        cmp = key_compare(user_data, key, p->data);
        if (!cmp && equal) {
            trav->current = p;
            return(p->data);
        }

        if (cmp > 0) {
            if (p->child[1] == NULL) {
                trav->current = p;
                return(p->data);
            }

            trav->stack[trav->height++] = p;
            p = p->child[1];
        } else {
            if (p->child[0] == NULL) {
                const bstnode_t *parent;

                if (trav->height > 0)
                    parent = trav->stack[--(trav->height)];
                else
                    parent = NULL;

                while (parent != NULL && p == parent->child[0]) {
                    p = parent;

                    if (trav->height > 0)
                        parent = trav->stack[--(trav->height)];
                    else
                        parent = NULL;
                }

                trav->current = parent;
                return((parent != NULL) ? parent->data : NULL);
            }

            trav->stack[trav->height++] = p;
            p = p->child[0];
        }
    }

    trav->height = 0;
    trav->current = NULL;
    return(NULL);
}

void *bstree_trav_lookup_higher (bstree_trav_t *trav,
                                 const void *key,
                                 int equal)
{
    compare_t key_compare;
    const bstnode_t *p;
    void *user_data;
    int cmp;

    trav->height = 0;
    trav->current = NULL;

    p = trav->tree->root;
    user_data = trav->tree->user_data;
    key_compare = trav->tree->info->key_compare;

    while (p != NULL) {
        cmp = key_compare(user_data, key, p->data);
        if (!cmp && equal) {
            trav->current = p;
            return(p->data);
        }

        if (cmp < 0) {
            if (p->child[0] == NULL) {
                trav->current = p;
                return(p->data);
            }

            trav->stack[trav->height++] = p;
            p = p->child[0];
        } else {
            if (p->child[1] == NULL) {
                const bstnode_t *parent;

                if (trav->height > 0)
                    parent = trav->stack[--(trav->height)];
                else
                    parent = NULL;

                while (parent != NULL && p == parent->child[1]) {
                    p = parent;

                    if (trav->height > 0)
                        parent = trav->stack[--(trav->height)];
                    else
                        parent = NULL;
                }

                trav->current = parent;
                return((parent != NULL) ? parent->data : NULL);
            }

            trav->stack[trav->height++] = p;
            p = p->child[1];
        }
    }

    trav->height = 0;
    trav->current = NULL;
    return(NULL);
}

void *bstree_trav_lookup_custom (bstree_trav_t *trav,
                                 const void *key,
                                 compare_t key_compare)
{
    const bstnode_t *p;
    void *user_data;
    int cmp;

    trav->height = 0;
    trav->current = NULL;
    user_data = trav->tree->user_data;

    p = trav->tree->root;
    while (p != NULL) {
        if (!(cmp = key_compare(user_data, key, p->data))) {
            trav->current = p;
            return(p->data);
        }

        p = p->child[cmp > 0];
    }

    trav->height = 0;
    trav->current = NULL;
    return(NULL);
}

