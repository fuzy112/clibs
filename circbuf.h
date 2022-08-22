/* circbuf.h
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

#ifndef CIRCBUF_H
#define CIRCBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct iovec;

struct circbuf {
    unsigned head;
    unsigned tail;
    unsigned size;
    unsigned _pad[1];
    uint8_t data[];
};

typedef struct circbuf *circbuf_ptr;

#define circbuf_autoptr circbuf_ptr __attribute__((cleanup(circ_cleanup)))

struct circbuf *circ_alloc(unsigned size);

void circ_free(struct circbuf *circ);

static inline void circ_cleanup(struct circbuf **circ)
{
    circ_free(*circ);
    *circ = NULL;
}

static inline bool circ_empty(const struct circbuf *circ)
{
    return circ->head == circ->tail;
}

static inline unsigned circ_count(const struct circbuf *circ)
{
    return (circ->tail - circ->head) & (circ->size - 1);
}

static inline unsigned circ_space(const struct circbuf *circ)
{
    return circ->size - 1 - circ_count(circ);
}

static inline unsigned circ_count_to_end(const struct circbuf *circ)
{
    unsigned end = circ->size - circ->tail;
    unsigned n = (circ->head + end) & (circ->size - 1);
    return n < end ? n : end;
}

static inline unsigned circ_space_to_end(struct circbuf *circ)
{
    unsigned end = circ->size - 1 - circ->head;
    unsigned n = (end + circ->tail) & (circ->size - 1);
    return n <= end ? n : end + 1;
}

void circ_prepare(struct circbuf *circ, struct iovec *iovecs,
                  unsigned *nr_vecs);

static inline void circ_commit(struct circbuf *circ, unsigned n)
{
    // assert(circ_space(circ) >= n);
    circ->tail += n;
}

void circ_data(struct circbuf *circ, struct iovec *iovecs, unsigned *nr_vecs);

static inline void circ_consume(struct circbuf *circ, unsigned n)
{
    // assert(circ_count(circ) >= n);
    circ->head += n;
}

int circ_read(struct circbuf *circ, void *__restrict buf, unsigned size,
              unsigned *off);

static inline int circ_read_u8(struct circbuf *circ, uint8_t *__restrict buf,
                               unsigned *off)
{
    return circ_read(circ, buf, 1, off);
}

static inline int circ_read_be16(struct circbuf *circ, uint16_t *buf,
                                 unsigned *off)
{
    uint8_t data[2] = {0};

    if (circ_read(circ, data, sizeof(data), off))
        return -1;

    *buf = (data[0] << 8) | (data[1] & 0xff);
    return 0;
}

static inline int circ_read_le16(struct circbuf *circ, uint16_t *buf,
                                 unsigned *off)
{
    uint8_t data[2] = {0};

    if (circ_read(circ, data, sizeof(data), off))
        return -1;

    *buf = data[0] | (data[1] << 8);
    return 0;
}

static inline int circ_read_be32(struct circbuf *circ, uint32_t *__restrict buf,
                                 unsigned *off)
{
    uint8_t data[4] = {0};

    if (circ_read(circ, data, sizeof(data), off))
        return -1;

    *buf = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return 0;
}

static inline int circ_read_le32(struct circbuf *circ, uint32_t *__restrict buf,
                                 unsigned *off)
{
    uint8_t data[4] = {0};

    if (circ_read(circ, data, sizeof(data), off))
        return -1;

    *buf = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    return 0;
}

int circ_write(struct circbuf *circ, const void *__restrict buf, unsigned size,
               unsigned *off);

static inline int circ_write_u8(struct circbuf *circ, unsigned value,
                                unsigned *off)
{
    uint8_t data[1];

    data[0] = value & 0xff;
    return circ_write(circ, data, sizeof(data), off);
}

static inline int circ_write_be16(struct circbuf *circ, unsigned value,
                                  unsigned *off)
{
    uint8_t data[2];

    data[0] = (value >> 8) & 0xff;
    data[1] = value & 0xff;

    return circ_write(circ, data, sizeof(data), off);
}

static inline int circ_write_le16(struct circbuf *circ, unsigned value,
                                  unsigned *off)
{
    uint8_t data[2];

    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;

    return circ_write(circ, data, sizeof(data), off);
}

static inline int circ_write_be32(struct circbuf *circ, uint_least32_t value,
                                  unsigned *off)
{
    uint8_t data[4] = {0};

    data[0] = (value >> 24) & 0xff;
    data[1] = (value >> 16) & 0xff;
    data[2] = (value >> 8) & 0xff;
    data[3] = value & 0xff;

    return circ_write(circ, data, sizeof(data), off);
}

static inline int circ_write_le32(struct circbuf *circ, uint_least32_t value,
                                  unsigned *off)
{
    uint8_t data[4] = {0};

    data[3] = (value >> 24) & 0xff;
    data[2] = (value >> 16) & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[0] = value & 0xff;

    return circ_write(circ, data, sizeof(data), off);
}

#endif // CIRCBUF_H
