#include "rbtree.h"
#include "tree2dot.h"
#include <stdlib.h>
#include <stdio.h>

struct rb_tree_node {
    struct rb_node node;

    int value;
};

static void rb_insert_node(struct rb_tree_node *node, struct rb_root *tree)
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &tree->rb_node;

    while (*link != NULL) {
        parent = *link;

        if (node->value < rb_entry(parent, struct rb_tree_node, node)->value)
            link = &parent->rb_left;
        else
            link = &parent->rb_right;
    }

    rb_link_node(&node->node, parent, link);
    rb_balance_insert(&node->node, tree);
}

static const void *rb_get_parent(const void *node)
{
    const struct rb_node *x = node;
    return x->rb_parent;
}

static const void *rb_get_left(const void *node)
{
    const struct rb_node *x = node;
    return x->rb_left;
}

static const void *rb_get_right(const void *node)
{
    const struct rb_node *x = node;
    return x->rb_right;
}

static int rb_get_label(const void *node, char *label)
{
    const struct rb_tree_node *x = node;

    if (x != NULL)
        return snprintf(label, T2D_LABEL_MAX, "%d", x->value);
    else
        return snprintf(label, T2D_LABEL_MAX, "NIL");
}

static const char *rb_get_color(const void *node)
{
    const struct rb_node *x = node;
    return (x == NULL || x->rb_is_black) ? "black" : "red";
}

static struct t2d_config rb_config = {
    .show_nil = true,
    .get_color = rb_get_color,
    .get_parent = rb_get_parent,
    .get_label = rb_get_label,
    .get_left = rb_get_left,
    .get_right = rb_get_right,
};

int main()
{
    struct rb_root tree = RB_ROOT_INIT;

    int v;

    rb_config.file = stdout;

    while (scanf("%d", &v) > 0) {
        struct rb_tree_node *node = malloc(sizeof(*node));
        if (!node) {
            perror("malloc");
            return 1;
        }

        node->value = v;

        rb_insert_node(node, &tree);
    }

    t2d_write_tree(&rb_config, tree.rb_node);

#ifdef __GNUC__
    rb_for_each_entry_safe_init (struct rb_tree_node, iter, &tree, node) {
        rb_erase(&iter->node, &tree);
        free(iter);
    }

#else
    {
        struct rb_node *iter, *n;

        rb_for_each_safe (iter, n, &tree) {
            struct rb_tree_node *node =
                rb_entry(iter, struct rb_tree_node, node);
            rb_erase(iter, &tree);
            free(node);
        }
    }

#endif
}
