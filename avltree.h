/* avltree.h
 *
 * Copyright 2022 Zhengyi Fu <tsingyat@outlook.com>
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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef container_of
#define container_of(ptr, type, member)                                        \
    ((type *)((char *)ptr - offsetof(type, member)))
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

#define avl_end(tree) ((struct avl_node *)NULL)
struct avl_node *avl_first(const struct avl_root *tree);
struct avl_node *avl_last(const struct avl_root *tree);
struct avl_node *avl_max(const struct avl_node *x);
struct avl_node *avl_min(const struct avl_node *x);
struct avl_node *avl_next(const struct avl_node *x);
struct avl_node *avl_prev(const struct avl_node *x);

static inline bool avl_empty(const struct avl_root *tree)
{
    return tree->avl_node == NULL;
}

static inline struct avl_node *avl_next_safe(const struct avl_node *n)
{
    return (n == NULL) ? NULL : avl_next(n);
}

#define avl_for_each(pos, tree)                                                \
    for ((pos) = avl_first((tree)); (pos) != NULL; (pos) = avl_next((pos)))

#define avl_for_each_safe(pos, n, tree)                                        \
    for ((pos) = avl_first((tree)), (n) = avl_next_safe((pos)); (pos) != NULL; \
         (pos) = (n), (n) = avl_next_safe((pos)))

#define avl_entry(ptr, type, member) container_of(ptr, type, member)

#if __STDC_VERSION__ >= 199901L || __cplusplus

#define avl_for_each_init(pos, tree)                                           \
    for (struct avl_node *pos = avl_first((tree)); pos != NULL;                \
         pos = avl_next((pos)))

#define avl_for_each_safe_init(pos, tree)                                      \
    for (struct avl_node *pos = avl_first((tree)),                             \
                         *_avl_next = avl_next_safe((pos), (tree));            \
         pos != NULL;                                                          \
         (pos) = _avl_next, _avl_next = avl_next_safe((pos), (tree)))

#endif

void avl_link_node(struct avl_node *node, struct avl_node *parent,
                   struct avl_node **link);
void avl_balance_insert(struct avl_node *node, struct avl_root *tree);

void avl_erase(struct avl_node *node, struct avl_root *tree);

#ifdef __cplusplus
}
#endif

#endif // !AVLTREE_H
