/* quark.c
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

#include "quark.h"
#include "rbtree.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct rb_root str_root = RB_ROOT_INIT;
static struct rb_root idx_root = RB_ROOT_INIT;

static long s_idx = 0;

struct quark_entry {
    struct rb_node str_node;
    struct rb_node idx_node;
    long idx;
    char str[];
};

void quark_init() {}

void quark_exit()
{
    rb_for_each_entry_safe_init(struct quark_entry, entry, &str_root, str_node)
    {
        rb_remove(&entry->str_node, &str_root);
        free(entry);
    }
}

static struct quark_entry *alloc_entry(const char *str)
{
    size_t str_len = strlen(str);
    size_t entry_size = sizeof(struct quark_entry) + str_len + 1;
    struct quark_entry *entry = malloc(entry_size);
    if (!entry)
        return NULL;
    entry->idx = ++s_idx;
    memcpy(entry->str, str, str_len + 1);
    return entry;
}

static struct quark_entry *insert_or_find_entry(const char *str)
{
    struct rb_node *parent = rb_end(&str_root);
    struct rb_node **link = &str_root.rb_node;
    int c;
    struct quark_entry *entry;

    while (*link != NULL) {
        parent = *link;
        c = strcmp(str, rb_entry(parent, struct quark_entry, str_node)->str);
        if (c < 0)
            link = &parent->rb_left;
        else if (c > 0)
            link = &parent->rb_right;
        else
            return rb_entry(parent, struct quark_entry, str_node);
    }

    entry = alloc_entry(str);
    rb_link_node(&entry->str_node, parent, link);
    rb_balance_insert(&entry->str_node, &str_root);

    parent = rb_end(&idx_root);
    link = &idx_root.rb_node;

    while (*link != NULL) {
        parent = *link;
        if (entry->idx < rb_entry(parent, struct quark_entry, idx_node)->idx)
            link = &parent->rb_left;
        else
            link = &parent->rb_right;
    }
    rb_link_node(&entry->idx_node, parent, link);
    rb_balance_insert(&entry->idx_node, &idx_root);

    return entry;
}

quark_t quark_from_str(const char *str)
{
    struct quark_entry *entry = insert_or_find_entry(str);
    if (!entry)
        return 0;
    return entry->idx;
}

const char *quark_intern(const char *str)
{
    struct quark_entry *entry = insert_or_find_entry(str);
    if (!entry)
        return 0;
    return entry->str;
}

const char *quark_to_str(quark_t quark)
{
    struct rb_node *parent = rb_end(&idx_root);
    struct rb_node **link = &idx_root.rb_node;
    int c;

    while (*link != NULL) {
        parent = *link;
        c = quark - rb_entry(parent, struct quark_entry, idx_node)->idx;
        if (c < 0)
            link = &parent->rb_left;
        else if (c > 0)
            link = &parent->rb_right;
        else
            return rb_entry(parent, struct quark_entry, idx_node)->str;
    }

    assert(quark == 0);
    return NULL;
}
