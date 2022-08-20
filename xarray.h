#ifndef XARRAY_H
#define XARRAY_H

#include <stddef.h>
#include <stdint.h>

union xa_slot {
    void *xa_ptr;
    struct xa_node *xa_node;
};

struct xarray {
    union xa_slot xa_slot;
    unsigned long xa_node_num;
    uint8_t xa_levels;
};


#define XA_BITS (sizeof(void*) == 8 ? 6 : 4)
#define XA_SLOT_MAX (1lu << 6)
#define XA_MASK (XA_SLOT_MAX - 1)

struct xa_node {
    uint8_t xa_shift;
    uint8_t xa_offset;
    uint8_t xa_count;              /* non null xa_slots */
    unsigned long xa_values;       /* xa_values in subtree */
    struct xa_node *xa_parent;
    union xa_slot xa_slots[XA_SLOT_MAX];
};

#define XA_FAILED ((void*)-1)

#define XA_INIT                                                                \
    {                                                                          \
        {NULL}, 0, 0                                                           \
    }

static inline void xa_init(struct xarray *xa)
{
    xa->xa_slot.xa_node = NULL;
    xa->xa_node_num = 0;
    xa->xa_levels = 0;
}

void xa_destroy(struct xarray *xa);

void *xa_store(struct xarray *xa, unsigned long index, void *item);

static inline void *xa_erase(struct xarray *xa, unsigned long index)
{
    return xa_store(xa, index, NULL);
}

void *xa_load(struct xarray *xa, unsigned long index);

unsigned long xa_size(const struct xarray *xa);

void xa_release(struct xarray *xa);


struct xa_node *xa_get_node_by_index(struct xarray *xa,
                                    unsigned long index);
struct xa_node *xa_find_node_by_index(struct xarray *xa,
                                    unsigned long index);

#endif // XARRAY_H
