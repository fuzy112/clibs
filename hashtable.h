#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "hash.h"
#include "ilog2.h"
#include "list.h"

#define HASHTABLE(name, bits) struct hlist_head name[1 << bits] = {0}

static inline void hashtable_init(struct hlist_head *table, unsigned int bits)
{
    int i;

    for (i = 0; i < (1 << bits); ++i) {
        INIT_HLIST_HEAD(&table[i]);
    }
}

#define HASHTABLE_SIZE(table) (sizeof(table) / sizeof((table)[0]))

#define HASHTABLE_BITS(table) ilog2_u32(HASHTABLE_SIZE(table))

#define htable_bucket_head(table, key)                                         \
    (&(table)[hash_long((key), HASHTABLE_BITS(table))])

#define htable_add(table, key, node)                                           \
    hlist_add_head(node, htable_bucket_head(table, key))

#define htable_for_each_possible(table, key, pos)                              \
    hlist_for_each (pos, htable_bucket_head(table, key))

#define htable_for_each_possible_safe(table, key, pos, n)                      \
    hlist_for_each_safe (pos, n, htable_bucket_head(table, key))

#define htable_for_each_possible_entry(table, key, pos, member)                \
    hlist_for_each_entry (pos, htable_bucket_head(table, key), member)

#define htable_for_each_possible_entry_safe(table, key, pos, n, member)        \
    hlist_for_each_entry_safe (pos, n, htable_bucket_head(table, key), member)

#define htable_for_each(table, bucket, pos)                                    \
    for (bucket = 0, pos = NULL;                                               \
         bucket < HASHTABLE_SIZE(table) && pos == NULL; ++bucket)              \
        hlist_for_each (pos, &(table)[bucket])

#define htable_for_each_safe(table, bucket, pos, n)                            \
    for (bucket = 0, pos = NULL;                                               \
         bucket < HASHTABLE_SIZE(table) && pos == NULL; ++bucket)              \
        hlist_for_each_safe (pos, n, &(table)[bucket])

#define htable_for_each_entry(table, bucket, pos, member)                      \
    for (bucket = 0, pos = NULL;                                               \
         bucket < HASHTABLE_SIZE(table) && pos == NULL; ++bucket)              \
        hlist_for_each_entry (pos, &(table)[bucket], member)

#define htable_for_each_entry_safe(table, bucket, pos, n, member)              \
    for (bucket = 0, pos = NULL;                                               \
         bucket < HASHTABLE_SIZE(table) && pos == NULL; ++bucket)              \
        hlist_for_each_entry_safe (pos, n, &(table)[bucket], member)

#endif /* HASHTABLE_H */
