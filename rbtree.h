/* rbtree.h
 *
 * Copyright 2022-2023 Zhengyi Fu <tsingyat@outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RBTREE_H
#define RBTREE_H

#include <stdbool.h>
#include <stddef.h>

#include "container_of.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rb_root {
    struct rb_node *rb_node;
};

struct rb_node {
    struct rb_node *rb_left;
    struct rb_node *rb_right;
    struct rb_node *rb_parent;
    bool rb_is_black;
};

// clang-format off
#define RB_ROOT_INIT   { NULL }
// clang-format on


static inline void rb_root_init(struct rb_root *tree) { tree->rb_node = NULL; }

static inline struct rb_node *rb_min(const struct rb_node *x)
{
    struct rb_node *y = NULL;
    while (x != NULL) {
        y = (struct rb_node *)x;
        x = x->rb_left;
    }
    return y;
}

static inline struct rb_node *rb_max(const struct rb_node *x)
{
    struct rb_node *y = NULL;
    while (x != NULL) {
        y = (struct rb_node *)x;
        x = x->rb_right;
    }
    return y;
}

/* Precondition: x != rb_first(root) */
static inline struct rb_node *rb_prev(const struct rb_node *x)
{
    struct rb_node *p;

    if (x == NULL)
        return NULL;

    if (x->rb_left != NULL)
        return rb_max(x->rb_left);

    p = x->rb_parent;
    while (p && p->rb_left == x) {
        x = p;
        p = x->rb_parent;
    }

    return p;
}

static inline struct rb_node *rb_next(const struct rb_node *x)
{
    struct rb_node *p;

    if (x == NULL)
        return NULL;

    if (x->rb_right != NULL)
        return rb_min(x->rb_right);

    p = x->rb_parent;
    while (p && p->rb_right == x) {
        x = p;
        p = x->rb_parent;
    }

    return p;
}


static inline struct rb_node *rb_first(const struct rb_root *tree)
{
    return rb_min(tree->rb_node);
}

static inline struct rb_node *rb_last(const struct rb_root *tree)
{
    return rb_max(tree->rb_node);
}


/* Link node x to parent. */
static inline void rb_link_node(struct rb_node *x, struct rb_node *parent,
                  struct rb_node **link)
{
    *link = x;
    x->rb_parent = parent;
    x->rb_left = x->rb_right = NULL;
    x->rb_is_black = false;
}


static inline bool rb_empty(const struct rb_root *tree)
{
    return tree->rb_node == NULL;
}

#define rb_for_each(pos, tree)                                                 \
    for ((pos) = rb_first(tree); (pos) != NULL; (pos) = rb_next((pos)))

#define rb_for_each_safe(pos, n, tree)                                         \
    for ((void)(((pos) = rb_first(tree)) && ((n) = rb_next(pos)));             \
         (pos) != NULL; (void)(((pos) = (n)) && ((n) = rb_next(pos))))

#define rb_entry(ptr, type, member) container_of(ptr, type, member)

#define rb_entry_safe(ptr, type, member)                                       \
    ((ptr) ? rb_entry(ptr, type, member) : NULL)

#define rb_first_entry(type, tree, member)                                     \
    (rb_entry_safe(rb_first(tree), type, member))

#define rb_next_entry(pos, member)                                             \
    rb_entry_safe(rb_next(&(pos)->member), typeof(*pos), member)

#define rb_for_each_entry(pos, tree, member)                                   \
    for ((pos) = rb_first_entry(typeof(*pos), tree, member); (pos) != NULL;    \
         (pos) = rb_next_entry(pos, member))

#define rb_for_each_entry_safe(pos, n, tree, member)                           \
    for ((void)(((pos) = rb_first_entry(typeof(*pos), tree, member)) &&        \
                ((n) = rb_next_entry(pos, member)));                           \
         (pos); (void)(((pos) = (n)) && ((n) = rb_next_entry(pos, member))))

void rb_balance_insert(struct rb_node *x, struct rb_root *root);

void rb_erase(struct rb_node *x, struct rb_root *root);

void rb_replace_node(struct rb_node *old, struct rb_node *new_node,
                     struct rb_root *tree);

static inline void rb_add(struct rb_node *x, struct rb_root *root,
                          bool (*less)(const struct rb_node *,
                                       const struct rb_node *))
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &parent->rb_left;

    while (*link != NULL) {
        parent = *link;

        if (less(x, parent))
            link = &parent->rb_left;
        else
            link = &parent->rb_right;
    }

    rb_link_node(x, parent, link);
    rb_balance_insert(x, root);
}

static inline struct rb_node *rb_find(const void *key, struct rb_root *root,
                                      int (*comp)(const void *,
                                                  const struct rb_node *))
{
    struct rb_node *node = root->rb_node;
    while (node != NULL) {
        int c = comp(key, node);

        if (c < 0)
            node = node->rb_left;
        else if (c > 0)
            node = node->rb_right;
        else
            return node;
    }
    return NULL;
}

static inline struct rb_node *
rb_find_or_insert(const void *key, struct rb_root *root, struct rb_node *node,
                  int (*comp)(const void *, const struct rb_node *))
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &root->rb_node;
    int c;

    while (*link != NULL) {
        parent = *link;
        c = comp(key, parent);
        if (c < 0)
            link = &parent->rb_left;
        else if (c > 0)
            link = &parent->rb_right;
        else
            return parent;
    }

    rb_link_node(node, parent, link);
    rb_balance_insert(node, root);
    return NULL;
}

#ifdef __cplusplus
}
#endif

#endif // !RBTREE_H
