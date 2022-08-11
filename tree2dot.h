#ifndef TREE2DOT_H
#define TREE2DOT_H

#include <stdbool.h>
#include <stdio.h>

#define T2D_LABEL_MAX 32

struct t2d_config {
    FILE *file;
    bool show_nil;
    const void *(*get_parent)(const void *);
    const void *(*get_left)(const void *);
    const void *(*get_right)(const void *);
    const char *(*get_color)(const void *);
    int (*get_label)(const void *, char *);
};

typedef unsigned t2d_node_id_t;

t2d_node_id_t t2d_write_node(struct t2d_config *t, const void *node);
int t2d_write_tree(struct t2d_config *t, const void *root);

#endif // TREE2DOT_H
