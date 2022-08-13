/* splay.h
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

#ifndef SPLAY_H
#define SPLAY_H

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

#ifndef __may_alias
#if defined(_MSC_VER) && !defined(__clang__)
#define __may_alias
#else
#define __may_alias __attribute__((__may_alias__))
#endif
#endif

struct splay_node {
    struct splay_node *splay_left;
    struct splay_node *splay_right;
    struct splay_node *splay_parent;
};

struct __may_alias splay_root {
    struct splay_node *splay_node;
};

// clang-format off
#define SPLAY_ROOT_INIT   { NULL }
// clang-format on

static inline void splay_root_init(struct splay_root *tree)
{
    tree->splay_node = NULL;
}

void splay_link_node(struct splay_node *splay_node,
                     struct splay_node *splay_parent,
                     struct splay_node **splay_link);

#define splay_end(tree) ((struct splay_node *)tree)
struct splay_node *splay_first(const struct splay_root *tree);
struct splay_node *splay_last(const struct splay_root *tree);
struct splay_node *splay_max(const struct splay_node *x);
struct splay_node *splay_min(const struct splay_node *x);
struct splay_node *splay_next(const struct splay_node *x);
struct splay_node *splay_prev(const struct splay_node *x);

static inline bool splay_empty(const struct splay_root *tree)
{
    return tree->splay_node == NULL;
}

static inline struct splay_node *splay_next_safe(const struct splay_node *n,
                                                 const struct splay_root *tree)
{
    return (n == splay_end(tree)) ? NULL : splay_next(n);
}

#define splay_for_each(pos, tree)                                              \
    for ((pos) = splay_first((tree)); (pos) != splay_end((tree));              \
         (pos) = splay_next((pos)))

#define splay_for_each_safe(pos, n, tree)                                      \
    for ((pos) = splay_first((tree)), (n) = splay_next_safe((pos), (tree));    \
         (pos) != splay_end((tree));                                           \
         (pos) = (n), (n) = splay_next_safe((pos), (tree)))

#define splay_entry(ptr, type, member) container_of(ptr, type, member)

#ifdef __GNUC__
#define splay_entry_safe(ptr, type, member)                                    \
    ({                                                                         \
        typeof(ptr) _splay_ptr = ptr;                                          \
        _splay_ptr ? splay_entry(_splay_ptr, type, member) : NULL;             \
    })
#else
static inline void *__splay_entry_safe(void *ptr, ptrdiff_t offset)
{
    return ptr ? (char *)ptr - offset : NULL;
}
#define splay_entry_safe(ptr, type, member)                                    \
    ((type *)__splay_entry_safe((void *)(ptr), offsetof(type, member)))
#endif

#define splay_first_entry(tree, type, member)                                  \
    splay_entry(splay_first((tree)), type, member)

#define splay_last_entry(tree, type, member)                                   \
    splay_entry(splay_last((tree)), type, member)

#define splay_next_entry(pos, member)                                          \
    splay_entry(splay_next(&(pos)->member), typeof(*pos), member)

#define splay_next_entry_safe(pos, tree, member)                               \
    splay_entry_safe(splay_next_safe(&(pos)->member, (tree)), typeof(*pos),    \
                     member)

#define splay_for_each_entry(pos, tree, member)                                \
    for ((pos) = splay_first_entry((tree), typeof(*pos), member);              \
         &(pos)->member != splay_end((tree));                                  \
         (pos) = splay_next_entry((pos), member))

#define splay_for_each_entry_safe(pos, n, tree, member)                        \
    for ((pos) = splay_first_entry((tree), typeof(*pos), member),              \
        (n) = splay_next_entry_safe((pos), (tree), member);                    \
         &(pos)->member != splay_end((tree));                                  \
         (pos) = (n), (n) = splay_next_entry_safe((pos), (tree), member))

#if __STDC_VERSION__ >= 199901L || __cplusplus

#define splay_for_each_init(pos, tree)                                         \
    for (struct splay_node *pos = splay_first((tree));                         \
         pos != splay_end((tree)); pos = splay_next((pos)))

#define splay_for_each_safe_init(pos, tree)                                    \
    for (struct splay_node *pos = splay_first((tree)),                         \
                           *_splay_next = splay_next_safe((pos), (tree));      \
         pos != splay_end((tree));                                             \
         (pos) = _splay_next, _splay_next = splay_next_safe((pos), (tree)))

#define splay_for_each_entry_init(type, pos, tree, member)                     \
    for (type *pos = splay_first_entry((tree), type, member);                  \
         &pos->member != splay_end((tree));                                    \
         pos = splay_next_entry(pos, member))

#define splay_for_each_entry_safe_init(type, pos, tree, member)                \
    for (type *pos = splay_first_entry((tree), type, member),                  \
              *_splay_next = splay_next_entry_safe(pos, (tree), member);       \
         &pos->member != splay_end((tree)); pos = _splay_next,                 \
              _splay_next = splay_next_entry_safe(pos, (tree), member))

#endif

/* Should be called after each search. */
void splay_balance(struct splay_node *node, struct splay_root *tree);

void splay_erase(struct splay_node *node, struct splay_root *tree);

#ifdef __cplusplus
}
#endif

#endif // SPLAY_H
