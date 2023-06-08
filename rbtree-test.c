#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct my_node {
    int value;
    struct rb_node node;
};

void add_my_node(struct my_node *n, struct rb_root *tree)
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &tree->rb_node;

    while (*link != NULL) {
        parent = *link;

        if (n->value < rb_entry(parent, struct my_node, node)->value)
            link = &parent->rb_left;
        else
            link = &parent->rb_right;
    }

    rb_link_node(&n->node, parent, link);
    rb_balance_insert(&n->node, tree);
}

int main()
{
    struct rb_root tree = RB_ROOT_INIT;
    struct rb_node *iter, *n;
    int i;
    struct my_node *pos, *tmp;

    srand(time(NULL));

    for (i = 0; i < 2000; ++i) {
        struct my_node *mynode = (struct my_node *)malloc(sizeof(*mynode));
        mynode->value = (unsigned)(rand() * rand());
        add_my_node(mynode, &tree);
    }

    rb_for_each_safe (iter, n, &tree) {
        struct my_node *entry = rb_entry(iter, struct my_node, node);
        if (entry->value % 2 == 0) {
            rb_erase(iter, &tree);
            free(entry);
        }
    }

    rb_for_each (iter, &tree) {
        printf("%d\n", rb_entry(iter, struct my_node, node)->value);
    }

    rb_for_each_entry (pos, &tree, node) {
        printf("%d\n", pos->value);
    }

    rb_for_each_entry_safe (pos, tmp, &tree, node) {
        pos = NULL;
    }

    rb_for_each_safe (iter, n, &tree) {
        struct my_node *entry = rb_entry(iter, struct my_node, node);

        rb_erase(iter, &tree);
        free(entry);
        entry = NULL;
    }

    rb_for_each_entry_safe (iter, n, &tree) {
         xa_for_each
    }

    return 0;
}
