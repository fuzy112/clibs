/* dict.c
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

#include "dict.h"
#include "rbtree.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct dict_entry {
    struct rb_node dict_node;
    void *dict_value;
    char dict_key[];
};

struct dict {
    void (*dict_free_value)(void *);
    void *(*dict_clone_value)(const void *);
    struct rb_root dict_tree;
};

static struct dict *dict_alloc(void (*free)(void *),
                               void *(*clone)(const void *))
{
    struct dict *retval = malloc(sizeof(*retval));
    if (retval == NULL)
        return NULL;
    retval->dict_free_value = free;
    retval->dict_clone_value = clone;
    rb_root_init(&retval->dict_tree);
    return retval;
}

int dict_create(struct dict **dict, void (*free_value)(void *),
                void *(*clone_value)(const void *))
{
    if (!dict)
        return -EINVAL;
    *dict = dict_alloc(free_value, clone_value);
    if (*dict == NULL)
        return -ENOMEM;
    return 0;
}

void dict_destroy(struct dict *dict)
{
    struct rb_node *iter, *n;
    rb_for_each_safe (iter, n, &dict->dict_tree) {
        struct dict_entry *entry = rb_entry(iter, struct dict_entry, dict_node);
        rb_erase(&entry->dict_node, &dict->dict_tree);
        dict->dict_free_value(entry->dict_value);
        free(entry);
    }
    free(dict);
}

static struct dict_entry *alloc_entry(struct dict *dict, const char *key,
                                      const void *value)
{
    size_t key_size = strlen(key);
    size_t entry_size = sizeof(struct dict_entry) + key_size + 1;
    struct dict_entry *entry = malloc(entry_size);
    if (!entry)
        return NULL;
    memcpy(entry->dict_key, key, key_size + 1);
    entry->dict_value = dict->dict_clone_value(value);
    if (entry->dict_value == NULL) {
        free(entry);
        return NULL;
    }

    return entry;
}

static void remove_entry(struct dict *dict, struct dict_entry *entry)
{
    rb_erase(&entry->dict_node, &dict->dict_tree);
    dict->dict_free_value(entry->dict_value);
    free(entry);
}

static int update_value(struct dict *dict, struct dict_entry *entry,
                        const void *value)
{
    void *new_value;
    if (value == NULL) {
        remove_entry(dict, entry);
        return 0;
    }
    new_value = dict->dict_clone_value(value);
    if (new_value == NULL)
        return -1;
    dict->dict_free_value(entry->dict_value);
    entry->dict_value = new_value;
    return 0;
}

int dict_set(struct dict *dict, const char *key, const void *value)
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &dict->dict_tree.rb_node;
    int c;
    struct dict_entry *entry;

    while (*link != NULL) {
        parent = *link;
        c = strcmp(key,
                   rb_entry(parent, struct dict_entry, dict_node)->dict_key);
        if (c < 0) {
            link = &parent->rb_left;
        } else if (c > 0) {
            link = &parent->rb_right;
        } else {
            return update_value(
                dict, rb_entry(parent, struct dict_entry, dict_node), value);
        }
    }

    entry = alloc_entry(dict, key, value);
    if (!entry)
        return -1;

    rb_link_node(&entry->dict_node, parent, link);
    rb_balance_insert(&entry->dict_node, &dict->dict_tree);
    return 0;
}

void *dict_get(struct dict *dict, const char *key)
{
    struct rb_node *node = dict->dict_tree.rb_node;
    while (node != NULL) {
        int c =
            strcmp(key, rb_entry(node, struct dict_entry, dict_node)->dict_key);

        if (c < 0)
            node = node->rb_left;
        else if (c > 0)
            node = node->rb_right;
        else
            return rb_entry(node, struct dict_entry, dict_node)->dict_value;
    }
    return NULL;
}
