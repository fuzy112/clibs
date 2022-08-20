#include "list.h"

#include <stdio.h>
#include <stdlib.h>

struct my_node {
    int value;
    struct list_head list;
};

int main()
{
    struct my_node *iter, *n;

    LIST_HEAD(list);

    // srand(time(NULL));

    for (int i = 0; i < 2000; ++i) {
        struct my_node *n = (struct my_node *)malloc(sizeof(*n));
        n->value = i % 23 + i % 27;
        list_add_tail(&n->list, &list);
    }

    list_for_each_entry_safe (iter, n, &list, list) {
        if (iter->value % 2 == 0) {
            list_del(&iter->list);
            free(iter);
        }
    }

    list_for_each_entry_safe (iter, n, &list, list) {
        printf("%d\n", iter->value);
    }

    list_for_each_entry_safe (iter, n, &list, list) {
        list_del(&iter->list);
        free(iter);
    }
}