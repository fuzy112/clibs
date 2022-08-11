#include "avltree.h"
#include "tree2dot.h"
#include <stdlib.h>
#include <stdio.h>

struct avl_tree_node {
    struct avl_node node;

    int value;
};

static void avl_insert_node(struct avl_tree_node *node, struct avl_root *tree)
{
    struct avl_node *parent = avl_end(tree);
    struct avl_node **link = &tree->avl_node;

    while (*link != NULL) {
        parent = *link;

        if (node->value < avl_entry(parent, struct avl_tree_node, node)->value)
            link = &parent->avl_left;
        else
            link = &parent->avl_right;
    }

    avl_link_node(&node->node, parent, link);
    avl_balance_insert(&node->node, tree);
}

static const void *avl_get_parent(const void *node)
{
    const struct avl_node *x = node;
    return x->avl_parent;
}

static const void *avl_get_left(const void *node)
{
    const struct avl_node *x = node;
    return x->avl_left;
}

static const void *avl_get_right(const void *node)
{
    const struct avl_node *x = node;
    return x->avl_right;
}

static int avl_get_label(const void *node, char *label)
{
    const struct avl_tree_node *x = node;
    return snprintf(label, T2D_LABEL_MAX, "%d", x->value);
}

static const char *avl_get_color(const void *node) { return "skyblue"; }

static struct t2d_config avl_config = {
    .show_nil = false,
    .get_color = avl_get_color,
    .get_parent = avl_get_parent,
    .get_label = avl_get_label,
    .get_left = avl_get_left,
    .get_right = avl_get_right,
};

int main()
{
    struct avl_root tree = AVL_ROOT_INIT;

    int v;

    avl_config.file = stdout;

    while (~scanf("%d", &v)) {
        struct avl_tree_node *node = malloc(sizeof(*node));
        if (!node) {
            perror("malloc");
            return 1;
        }

        node->value = v;

        avl_insert_node(node, &tree);
    }

    t2d_write_tree(&avl_config, tree.avl_node);

    avl_for_each_entry_safe_init(struct avl_tree_node, iter, &tree, node)
    {
        avl_erase(&iter->node, &tree);
        free(iter);
    }
}
