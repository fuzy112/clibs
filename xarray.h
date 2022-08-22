#ifndef XARRAY_H
#define XARRAY_H

#include <stddef.h>
#include <stdint.h>

struct xarray {
    void *xa_slot;
    unsigned long xa_node_num;
    uint8_t xa_levels;
};

#define XA_BITS (sizeof(void *) == 8 ? 6 : 4)
#define XA_SLOT_MAX (1lu << 6)
#define XA_MASK (XA_SLOT_MAX - 1)

struct xa_node {
    uint8_t xa_shift;
    uint8_t xa_offset;
    uint8_t xa_count;        /* non null xa_slots */
    unsigned long xa_values; /* values in subtree */
    struct xa_node *xa_parent;
    void *xa_slots[XA_SLOT_MAX];
};

#define XA_FAILED ((void *)-1)

#define XA_INIT                                                                \
    {                                                                          \
        NULL, 0, 0                                                             \
    }

static inline void xa_init(struct xarray *xa)
{
    xa->xa_slot = NULL;
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

struct xa_node *xa_get_node_by_index(struct xarray *xa, unsigned long index);
struct xa_node *xa_find_node_by_index(struct xarray *xa, unsigned long index);

void *xa_find(struct xarray *xa, unsigned long *indexp, unsigned long last);

void *xa_find_after(struct xarray *xa, unsigned long *indexp,
                    unsigned long last);

#define xa_for_each_range(xa, index, value, start, end)                        \
    for ((index) = (start), (value) = xa_find((xa), &(index), (end));          \
         (value) != NULL; (value) = xa_find_after((xa), &(index), (end)))

#define xa_for_each(xa, index, value)                                          \
    xa_for_each_range(xa, index, value, 0, ULONG_MAX)

#endif // XARRAY_H
