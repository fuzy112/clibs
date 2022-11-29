#include "splay.h"
#include "tree2dot.h"
#include <stdio.h>
#include <stdlib.h>

struct splay_tree_node {
    struct splay_node node;

    int value;
};

static void splay_insert_node(struct splay_tree_node *node,
                              struct splay_root *tree)
{
    struct splay_node *parent = splay_end(tree);
    struct splay_node **link = &tree->splay_node;

    while (*link != NULL) {
        parent = *link;

        if (node->value <
            splay_entry(parent, struct splay_tree_node, node)->value)
            link = &parent->splay_left;
        else
            link = &parent->splay_right;
    }

    splay_link_node(&node->node, parent, link);
    splay_balance(&node->node, tree);
}

static const void *splay_get_parent(const void *node)
{
    const struct splay_node *x = node;
    return x->splay_parent;
}

static const void *splay_get_left(const void *node)
{
    const struct splay_node *x = node;
    return x->splay_left;
}

static const void *splay_get_right(const void *node)
{
    const struct splay_node *x = node;
    return x->splay_right;
}

static int splay_get_label(const void *node, char *label)
{
    const struct splay_tree_node *x = node;

    if (x != NULL)
        return snprintf(label, T2D_LABEL_MAX, "%d", x->value);
    else
        return snprintf(label, T2D_LABEL_MAX, "NIL");
}

static const char *splay_get_color(const void *node)
{
    (void)node;
    return "blue";
}

static struct t2d_config splay_config = {
    .show_nil = true,
    .get_color = splay_get_color,
    .get_parent = splay_get_parent,
    .get_label = splay_get_label,
    .get_left = splay_get_left,
    .get_right = splay_get_right,
};

int main()
{
    struct splay_root tree = SPLAY_ROOT_INIT;

    int v;

    splay_config.file = stdout;

    while (scanf("%d", &v) > 0) {
        struct splay_tree_node *node = malloc(sizeof(*node));
        if (!node) {
            perror("malloc");
            return 1;
        }

        node->value = v;

        splay_insert_node(node, &tree);
    }

    t2d_write_tree(&splay_config, tree.splay_node);

    {
        struct splay_node *iter, *n;

        splay_for_each_safe (iter, n, &tree) {
            struct splay_tree_node *node =
                splay_entry(iter, struct splay_tree_node, node);
            splay_erase(iter, &tree);
            free(node);
        }
    }
    return 0;
}
