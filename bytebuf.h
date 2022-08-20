#ifndef BYTEBUF_H
#define BYTEBUF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct bytebuf {
    char *data;
    size_t size;
    size_t capacity;
};

#define BYTEBUF_INIT \
    { NULL, 0, 0 }

static inline void bytebuf_init(struct bytebuf *b)
{
    b->data = NULL;
    b->size = 0;
    b->capacity = 0;
}

#endif // BYTEBUF_H
