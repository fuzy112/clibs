#include "xarray.h"
// #include "list.h"
#include "error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#define xa_slot_index(shift, index) \
    ((index >> shift) & XA_MASK)

static unsigned long xa_max_index(uint8_t levels)
{
    if (levels * XA_BITS > 8 * sizeof(unsigned long))
        return ULONG_MAX;
    return  ((1lu << (XA_BITS * (levels))) - 1lu);
}

static struct xa_node *xa_alloc_node(struct xarray *xa)
{
    struct xa_node *node = (struct xa_node *)calloc(1, sizeof(*node));
    if (node) {
        xa->xa_node_num++;
    }
    return node;
}

static int xa_increase_level(struct xarray *xa)
{
    struct xa_node *node = xa_alloc_node(xa);
    struct xa_node *old_root;
    if (!node)
        return -ENOMEM;

    old_root = node->xa_slots[0] = xa->xa_slot;
    if (old_root) {
        old_root->xa_parent = node;
        node->xa_values = old_root->xa_values;
    }
    xa->xa_slot = node;
    xa->xa_levels++;
    node->xa_shift = XA_BITS * (xa->xa_levels - 1);
    node->xa_count = 1;

    return 0;
}

struct xa_node *xa_get_leaf_by_index(struct xarray *xa,
                                              const unsigned long index)
{
    uint8_t i;
    void **slot;
    struct xa_node *xa_parent = NULL;

    while ((index != ULONG_MAX ? (index+1) : index) > xa_max_index(xa->xa_levels)) {
        if (xa_increase_level(xa))
            return NULL;
    }

    slot = &xa->xa_slot;
    for (i = 0; i < xa->xa_levels; ++i) {
        if (*slot == NULL) {
            struct xa_node *node = *slot = xa_alloc_node(xa);
            if (node == NULL)
                return NULL;
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

struct xa_node *xa_find_leaf_by_index(struct xarray *xa,
                                     const unsigned long index)
{
    uint8_t i;
    void **slot;
    struct xa_node *xa_parent = NULL;

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

void* xa_store(struct xarray *xa, unsigned long index, void *item)
{
    struct xa_node *node = xa_get_leaf_by_index(xa, index);
    void **slot;
    void *old_value;

    if (!node)
        return_val_err(XA_FAILED, ENOMEM);

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
    return 0;
}

void *xa_load(struct xarray *xa, unsigned long index)
{
    void **slot;
    struct xa_node *node = xa_find_leaf_by_index(xa, index);

    if (!node)
        return_val_err(XA_FAILED, ENOENT);
    
    slot = &node->xa_slots[index & XA_MASK];
    return *slot;
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

void xa_destroy(struct xarray *xa)
{
    xa_free_level(xa, 0, xa->xa_slot);
}

unsigned long xa_size(const struct xarray *xa)
{
    struct xa_node *root = xa->xa_slot;
    if (root) {
        return root->xa_values;
    }
    return 0;
}

static void xa_release_level(struct xarray *xa, uint8_t level, struct xa_node *node)
{
    if (!node)
        return;

    if (level < xa->xa_levels) {
        uint8_t i;
        for (i = 0; i < XA_SLOT_MAX; ++i) {
            xa_release_level(xa, level + 1, node->xa_slots[i]);
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
}

void xa_release(struct xarray *xa)
{
    xa_release_level(xa, 0, xa->xa_slot);
}
