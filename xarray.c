/* xarray.c
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

#include "xarray.h"
#include "error.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static inline uint8_t __attribute_pure__ xa_slot_index(uint8_t shift,
                                                       unsigned long index)
{
    return (uint8_t)(index >> shift) & XA_MASK;
}

static inline unsigned long __attribute_pure__ xa_max_index(uint8_t levels)
{
    if (levels * XA_BITS > 8 * sizeof(unsigned long))
        return XA_INDEX_MAX;
    return ((1lu << (XA_BITS * (levels))) - 1lu);
}

static struct xa_node *xa_alloc_node(struct xarray *xa)
{
    struct xa_node *node = (struct xa_node *)calloc(1, sizeof(*node));
    if (!node)
        return_null_err(ENOMEM);
    xa->xa_node_num++;
    return node;
}

static inline bool xa_is_leaf(const struct xa_node *node)
{
    assert(node);
    return node->xa_shift == 0;
}

static int xa_increase_level(struct xarray *xa)
{
    struct xa_node *node = xa_alloc_node(xa);
    struct xa_node *old_root;
    if (!node)
        return -1;

    old_root = node->xa_slots[0] = xa->xa_slot;
    if (old_root) {
        old_root->xa_parent = node;
        node->xa_values = old_root->xa_values;
    }
    xa->xa_slot = node;
    xa->xa_levels++;
    node->xa_shift = XA_BITS * (xa->xa_levels - 1);
    node->xa_count = old_root != NULL ? 1 : 0;

    return 0;
}

struct xa_node *xa_get_leaf_by_index(struct xarray *xa,
                                     const unsigned long index)
{
    uint8_t i;
    void **slot;
    struct xa_node *xa_parent = NULL;

    while ((index != XA_INDEX_MAX ? (index + 1) : index) >
           xa_max_index(xa->xa_levels)) {
        if (xa_increase_level(xa))
            return NULL;
    }

    slot = &xa->xa_slot;
    for (i = 0; i < xa->xa_levels; ++i) {
        if (*slot == NULL) {
            struct xa_node *node = *slot = xa_alloc_node(xa);
            if (node == NULL)
                return NULL;

            assert(xa_parent);
            node->xa_parent = xa_parent;
            node->xa_offset = xa_slot_index(xa_parent->xa_shift, index);
            node->xa_shift = xa_parent->xa_shift - XA_BITS;
            xa_parent->xa_count++;
        }

        xa_parent = *slot;
        slot = &xa_parent->xa_slots[xa_slot_index(xa_parent->xa_shift, index)];
    }

    return xa_parent;
}

struct xa_node *xa_get_leaf_with_space_by_index(struct xarray *xa,
                                                unsigned long *indexp,
                                                unsigned long last)
{
    struct xa_node *node;
    unsigned long index = *indexp;

    node = xa_get_leaf_by_index(xa, index);

    while (index <= last) {

        if (!node)
            return NULL;

        if (node->xa_slots[index & XA_MASK]) {
            index++;

            if (xa_slot_index(node->xa_shift, index) == 0)
                node = xa_get_leaf_by_index(xa, index);

        } else {
            *indexp = index;
            return node;
        }
    }

    return_null_err(EBUSY);
}

const struct xa_node *xa_find_leaf_by_index(const struct xarray *xa,
                                            const unsigned long index)
{
    uint8_t i;
    void *const *slot;
    const struct xa_node *xa_parent = NULL;

    if (index + 1 > xa_max_index(xa->xa_levels))
        return NULL;

    slot = &xa->xa_slot;
    for (i = 0; i < xa->xa_levels; ++i) {
        if (*slot == NULL)
            return NULL;

        xa_parent = *slot;
        slot = &xa_parent->xa_slots[xa_slot_index(xa_parent->xa_shift, index)];
    }

    return xa_parent;
}

void *xa_store(struct xarray *xa, unsigned long index, void *item)
{
    struct xa_node *node = xa_get_leaf_by_index(xa, index);
    void **slot;
    void *old_value;

    if (!node)
        return_val_err(item ? XA_FAILED : NULL, ENOMEM);

    slot = &node->xa_slots[index & XA_MASK];

    old_value = *slot;

    *slot = item;

    if (old_value && !item) {
        node->xa_count--;
        while (node != 0) {
            node->xa_values--;
            node = node->xa_parent;
        }
    }

    if (!old_value && item) {
        node->xa_count++;
        while (node != 0) {
            node->xa_values++;
            node = node->xa_parent;
        }
    }
    return old_value;
}

void *xa_load(const struct xarray *xa, unsigned long index)
{
    const struct xa_node *node = xa_find_leaf_by_index(xa, index);

    if (!node)
        return NULL;
    return node->xa_slots[index & XA_MASK];
}

static void xa_free_level(struct xarray *xa, unsigned level,
                          struct xa_node *node)
{
    if (!node)
        return;

    if (level < xa->xa_levels) {
        unsigned i;
        for (i = 0; i < XA_SLOT_MAX; ++i) {
            xa_free_level(xa, level + 1, node->xa_slots[i]);
        }

        free(node);
    }
}

void xa_destroy(struct xarray *xa) { xa_free_level(xa, 0, xa->xa_slot); }

unsigned long xa_size(const struct xarray *xa)
{
    struct xa_node *root = xa->xa_slot;
    if (root) {
        return root->xa_values;
    }
    return 0;
}

static void xa_release_level(struct xarray *xa, uint8_t level,
                             struct xa_node *node)
{
    uint8_t i;

    if (!node)
        return;

    if (level == xa->xa_levels)
        return;

    if (level + 1 < xa->xa_levels) {
        for (i = 0; i < XA_SLOT_MAX; ++i) {
            xa_release_level(xa, level + 1, node->xa_slots[i]);
        }
    }

    if (node->xa_count == 0) {
        xa->xa_node_num--;
        if (node->xa_parent) {
            node->xa_parent->xa_slots[node->xa_offset] = NULL;
            node->xa_parent->xa_count--;
        } else {
            xa->xa_slot = NULL;
        }
        free(node);
    }
}

void xa_release(struct xarray *xa)
{
    xa_release_level(xa, 0, xa->xa_slot);

    struct xa_node *node = xa->xa_slot;
    if (!node)
        return;

    while (node->xa_shift != 0 && node->xa_count == 1 &&
           node->xa_slots[0] != NULL) {
        node = node->xa_slots[0];
        assert(node != xa->xa_slot);
        free(xa->xa_slot);
        xa->xa_slot = node;
        xa->xa_node_num--;
        xa->xa_levels--;
        if (node) {
            node->xa_parent = NULL;
        }
    }
}

void *xa_find(struct xarray *xa, unsigned long *indexp, unsigned long last)
{
    unsigned long index = *indexp;
    struct xa_node *node = xa->xa_slot;

    if (xa_max_index(xa->xa_levels) < last)
        last = xa_max_index(xa->xa_levels);

    while (index <= last) {
        void *slot;

        slot = node->xa_slots[xa_slot_index(node->xa_shift, index)];

        if (slot) {
            if (xa_is_leaf(node)) {
                *indexp = index;
                return slot;
            }

            node = slot;
        } else {
            index += 1ul << node->xa_shift;

            while (node && xa_slot_index(node->xa_shift, index) == 0) {
                node = node->xa_parent;
            }

            if (!node)
                break;
        }
    }

    return NULL;
}

void *xa_find_after(struct xarray *xa, unsigned long *indexp,
                    unsigned long last)
{
    void *ret;
    unsigned long index = *indexp;

    if (index >= last)
        return NULL;

    index += 1;
    ret = xa_find(xa, &index, last);
    if (ret)
        *indexp = index;
    return ret;
}

int xa_insert(struct xarray *xa, unsigned long *indexp, void *item,
              unsigned long last)
{
    struct xa_node *node = xa_get_leaf_with_space_by_index(xa, indexp, last);
    if (!node)
        return -1;

    node->xa_slots[(*indexp) & XA_MASK] = item;
    node->xa_count++;
    while (node) {
        node->xa_values++;
        node = node->xa_parent;
    }
    return 0;
}
