#include "avltree.h"
#include <stdio.h>
#include <stdlib.h>

struct my_node {
    int value;
    struct avl_node node;
};

void add_my_node(struct my_node *n, struct avl_root *tree)
{
    struct avl_node *parent = avl_end(tree);
    struct avl_node **link = &tree->avl_node;

    while (*link != NULL) {
        parent = *link;

        if (n->value < avl_entry(parent, struct my_node, node)->value)
            link = &parent->avl_left;
        else
            link = &parent->avl_right;
    }

    avl_link_node(&n->node, parent, link);
    avl_balance_insert(&n->node, tree);
}

int main()
{
    struct avl_root tree = AVL_ROOT_INIT;

    // srand(time(NULL));

    for (int i = 0; i < 2000; ++i) {
        struct my_node *n = (struct my_node *)malloc(sizeof(*n));
        n->value = i % 23 + i % 27;
        add_my_node(n, &tree);
    }

    avl_for_each_entry_safe_init (struct my_node, iter, &tree, node) {
        if (iter->value % 2 == 0) {
            avl_erase(&iter->node, &tree);
            free(iter);
        }
    }

    avl_for_each_entry_init (struct my_node, iter, &tree, node) {
        printf("%d\n", iter->value);
    }

    avl_for_each_entry_safe_init (struct my_node, iter, &tree, node) {
        avl_erase(&iter->node, &tree);
        free(iter);
    }
}
