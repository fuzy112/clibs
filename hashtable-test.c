#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct my_type {
    int key;
    struct hlist_node node;
};

static HASHTABLE(my_table, 8);

int main()
{
    int i;
    struct my_type *t;
    struct hlist_node *n;

    for (i = 0; i < 200; ++i) {
        t = malloc(sizeof(*t));
        if (!t) {
            perror("malloc");
            abort();
        }

        t->key = i;
        htable_add(my_table, t->key, &t->node);
    }

    for (i = 0; i < 100; ++i) {
        struct hlist_node *pos;
        struct hlist_node *tmp;

        htable_for_each_possible_safe (my_table, i, pos, tmp) {
            hlist_del(pos);
            t = container_of(pos, struct my_type, node);
            printf("[%i] = %i\n", i, t->key);
            free(t);
        }
    }

    htable_for_each_entry_safe (my_table, i, t, n, node) {
        printf("bucket[%i] = %i\n", i, t->key);
        hlist_del(&t->node);
        free(t);
    }

    return 0;
}
