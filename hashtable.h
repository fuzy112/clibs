#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "hash.h"
#include "ilog2.h"
#include "list.h"

#define HASHTABLE(name, bits)                                                  \
    struct hlist_head name[1 << (bits)] = {[0 ...((1 << (bits)) - 1)] =        \
                                               HLIST_HEAD_INIT}

static inline void __hash_init(struct hlist_head *table, unsigned int size)
{
    unsigned int i;

    for (i = 0; i < size; ++i)
        INIT_HLIST_HEAD(&table[i]);
}

#define HASH_SIZE(table) (sizeof(table) / sizeof((table)[0]))

#define HASH_BITS(table) ilog2_u32(HASH_SIZE(table))

#define hash_init(table) __hash_init(table, HASH_SIZE(table))

#define hash_bucket_head(table, key)                                           \
    (&(table)[hash_long((key), HASH_BITS(table))])

#define hash_add(table, key, node)                                             \
    hlist_add_head(node, hash_bucket_head(table, key))

#define hash_for_each_possible(table, key, pos)                                \
    hlist_for_each (pos, hash_bucket_head(table, key))

#define hash_for_each_possible_safe(table, key, pos, n)                        \
    hlist_for_each_safe (pos, n, hash_bucket_head(table, key))

#define hash_for_each_possible_entry(table, key, pos, member)                  \
    hlist_for_each_entry (pos, hash_bucket_head(table, key), member)

#define hash_for_each_possible_entry_safe(table, key, pos, n, member)          \
    hlist_for_each_entry_safe (pos, n, hash_bucket_head(table, key), member)

#define hash_for_each(table, bucket, pos)                                      \
    for (bucket = 0, pos = NULL; bucket < HASH_SIZE(table) && pos == NULL;     \
         ++bucket)                                                             \
        hlist_for_each (pos, &(table)[bucket])

#define hash_for_each_safe(table, bucket, pos, n)                              \
    for (bucket = 0, pos = NULL; bucket < HASH_SIZE(table) && pos == NULL;     \
         ++bucket)                                                             \
        hlist_for_each_safe (pos, n, &(table)[bucket])

#define hash_for_each_entry(table, bucket, pos, member)                        \
    for (bucket = 0, pos = NULL; bucket < HASH_SIZE(table) && pos == NULL;     \
         ++bucket)                                                             \
        hlist_for_each_entry (pos, &(table)[bucket], member)

#define hash_for_each_entry_safe(table, bucket, pos, n, member)                \
    for (bucket = 0, pos = NULL; bucket < HASH_SIZE(table) && pos == NULL;     \
         ++bucket)                                                             \
        hlist_for_each_entry_safe (pos, n, &(table)[bucket], member)

#endif /* HASHTABLE_H */
