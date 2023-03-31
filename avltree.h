/* avltree.h
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

#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "container_of.h"

#ifdef __cplusplus
extern "C" {
#endif

struct avl_node {
    struct avl_node *avl_left;
    struct avl_node *avl_right;
    struct avl_node *avl_parent;
    int_fast8_t avl_balance;
};

struct avl_root {
    struct avl_node *avl_node;
};

// clang-format off
#define AVL_ROOT_INIT { NULL }
// clang-format on


static inline void avl_root_init(struct avl_root *tree)
{
    tree->avl_node = NULL;
}


static inline struct avl_node *avl_min(const struct avl_node *x)
{
    struct avl_node *y = NULL;
    while (x != NULL) {
        y = (struct avl_node *)x;
        x = x->avl_left;
    }
    return y;
}

static inline struct avl_node *avl_max(const struct avl_node *x)
{
    struct avl_node *y = NULL;
    while (x != NULL) {
        y = (struct avl_node *)x;
        x = x->avl_right;
    }
    return y;
}

static inline struct avl_node *avl_prev(const struct avl_node *x)
{
    struct avl_node *p;

    if (x == NULL)
        return NULL;

    if (x->avl_left != NULL)
        return avl_max(x->avl_left);

    p = x->avl_parent;
    while (p && p->avl_left == x) {
        x = p;
        p = x->avl_parent;
    }

    return p;
}

static inline struct avl_node *avl_next(const struct avl_node *x)
{
    struct avl_node *p;

    if (x == NULL)
        return NULL;

    if (x->avl_right != NULL)
        return avl_min(x->avl_right);

    p = x->avl_parent;
    while (p && p->avl_right == x) {
        x = p;
        p = x->avl_parent;
    }

    return p;
}

/* Link node x to parent. */
static inline void avl_link_node(struct avl_node *x, struct avl_node *parent,
                   struct avl_node **link)
{
    *link = x;
    x->avl_parent = parent;
    x->avl_left = x->avl_right = NULL;
    x->avl_balance = 0;
}


static inline struct avl_node *avl_first(const struct avl_root *tree)
{
    return avl_min(tree->avl_node);
}

static inline struct avl_node *avl_last(const struct avl_root *tree)
{
    return avl_max(tree->avl_node);
}


static inline bool avl_empty(const struct avl_root *tree)
{
    return tree->avl_node == NULL;
}

#define avl_for_each(pos, tree)                                                \
    for ((pos) = avl_first((tree)); (pos) != NULL; (pos) = avl_next((pos)))

#define avl_for_each_safe(pos, n, tree)                                        \
    for ((void)(((pos) = avl_first(tree)) && ((n) = avl_next(pos)));           \
         (pos) != NULL; (void)(((pos) = (n)) && ((n) = avl_next(pos))))

#define avl_entry(ptr, type, member) container_of(ptr, type, member)

#define avl_entry_safe(ptr, type, member)                                      \
    ((ptr) ? avl_entry(ptr, type, member) : NULL)

#define avl_first_entry(type, tree, member)                                    \
    avl_entry_safe(avl_first(tree), type, member)

#define avl_next_entry(pos, member)                                            \
    avl_entry_safe(avl_next(&(pos)->member), typeof(*pos), member)

#define avl_for_each_entry(pos, tree, member)                                  \
    for ((pos) = avl_first_entry(typeof(*pos), tree, member); (pos);           \
         (pos) = avl_next_entry(pos, member))

#define avl_for_each_entry_safe(pos, n, tree, member)                          \
    for ((void)(((pos) = avl_first_entry(typeof(*pos), tree, member)) &&       \
                ((n) = avl_next_entry(pos, member)));                          \
         (pos); (void)(((pos) = (n)) && ((n) = avl_next_entry(pos, member))))

void avl_link_node(struct avl_node *node, struct avl_node *parent,
                   struct avl_node **link);
void avl_balance_insert(struct avl_node *node, struct avl_root *tree);

void avl_erase(struct avl_node *node, struct avl_root *tree);

#ifdef __cplusplus
}
#endif

#endif // !AVLTREE_H
