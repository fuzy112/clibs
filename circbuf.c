#include "circbuf.h"
#include "error.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

void circ_prepare(struct circbuf *circ, struct iovec *iovecs, unsigned *nr_vecs)
{
    unsigned tail_i = circ->tail & (circ->size - 1);
    unsigned before_head_i = (circ->head - 1) & (circ->size - 1);

    if (tail_i == before_head_i) {
        *nr_vecs = 0;
        return;
    }

    if (tail_i > before_head_i) {
        iovecs[0].iov_base = &circ->data[tail_i];
        iovecs[0].iov_len = circ->size - tail_i;

        if (before_head_i == 0) {
            *nr_vecs = 1;
            return;
        }
        iovecs[1].iov_base = circ->data;
        iovecs[1].iov_len = before_head_i;

        *nr_vecs = 2;
    } else {
        iovecs[0].iov_base = &circ->data[tail_i];
        iovecs[0].iov_len = before_head_i - tail_i;
        *nr_vecs = 1;
        return;
    }
}

void circ_data(struct circbuf *circ, struct iovec *iovecs, unsigned *nr_vecs)
{
    unsigned tail_i = circ->tail & (circ->size - 1);
    unsigned head_i = circ->head & (circ->size - 1);

    if (tail_i == head_i) {
        *nr_vecs = 0;
        return;
    }

    if (head_i > tail_i) {
        iovecs[0].iov_base = &circ->data[head_i];
        iovecs[0].iov_len = circ->size - head_i;

        if (tail_i == 0) {
            *nr_vecs = 1;
            return;
        }

        iovecs[1].iov_base = circ->data;
        iovecs[1].iov_len = tail_i - 1;
        *nr_vecs = 2;
        return;
    } else {
        iovecs[0].iov_base = &circ->data[head_i];
        iovecs[0].iov_len = tail_i - head_i;
        *nr_vecs = 1;
        return;
    }
}

struct circbuf *circ_alloc(unsigned size)
{
    struct circbuf *circ;

    assert((size & (size - 1)) == 0);
    if (size == 0 || (size & (size - 1)) != 0) {
        return_null_err(EINVAL);
    }

    circ = (struct circbuf *)malloc(sizeof(struct circbuf) + size);
    if (!circ)
        return NULL;

    circ->size = size;
    circ->head = 0;
    circ->tail = 0;

    memset(circ->data, 0xff, size);

    return circ;
}

void circ_free(struct circbuf *circ) { free(circ); }

int circ_read(struct circbuf *circ, void *__restrict buf, unsigned size,
              unsigned *off)
{
    unsigned head = circ->head + *off;

    if (((circ->tail - head) & (circ->size - 1)) >= size) {
        unsigned count_to_end = circ_count_to_end(circ);
        memcpy(buf, &circ->data[head & (circ->size - 1)],
               count_to_end > size ? size : count_to_end);
        if (count_to_end < size) {
            memcpy((uint8_t *)buf + count_to_end, circ->data,
                   size - count_to_end);
        }
        *off += size;
        return 0;
    }

    return_fail_err(EIO);
}

int circ_write(struct circbuf *circ, const void *__restrict buf, unsigned size,
               unsigned *off)
{
    unsigned tail = circ->tail + *off;

    if (circ->size + circ->head - 1 - tail > size) {
        unsigned space_to_end = circ_space_to_end(circ);
        memcpy(&circ->data[tail], buf,
               space_to_end > size ? size : space_to_end);
        if (space_to_end < size) {
            memcpy(circ->data, (const uint8_t *)buf + space_to_end,
                   size - space_to_end);
        }
        *off += size;
        return 0;
    }
    return_fail_err(EIO);
}