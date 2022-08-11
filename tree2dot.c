#include "tree2dot.h"
#include <inttypes.h>

#define PRInode_id "node%04" PRIx32

static t2d_node_id_t alloc_id(void)
{
    static t2d_node_id_t counter;
    return ++counter;
}

t2d_node_id_t t2d_write_node(struct t2d_config *cfg, const void *node)
{
    const t2d_node_id_t id = alloc_id();
    char label[T2D_LABEL_MAX];

    if (node == NULL) {
        if (!cfg->show_nil)
            return 0;

        cfg->get_label(node, label);
        fprintf(cfg->file, PRInode_id "[label=%s, fillcolor=%s]\n", id, label,
                cfg->get_color(node));
    } else {
        t2d_node_id_t left_id = t2d_write_node(cfg, cfg->get_left(node));
        t2d_node_id_t right_id = t2d_write_node(cfg, cfg->get_right(node));

        cfg->get_label(node, label);
        fprintf(cfg->file, PRInode_id "[label=%s, fillcolor=%s]\n", id, label,
                cfg->get_color(node));
        if (left_id != 0)
            fprintf(cfg->file, PRInode_id " -> " PRInode_id "\n", id, left_id);

        if (right_id != 0)
            fprintf(cfg->file, PRInode_id " -> " PRInode_id "\n", id, right_id);
    }

    return id;
}

int t2d_write_tree(struct t2d_config *cfg, const void *root)
{
    fprintf(cfg->file, "digraph {\n");
    fprintf(cfg->file, "ordering=out\n");
    fprintf(cfg->file, "node[style=filled, shape=circle]\n");

    if (t2d_write_node(cfg, root) == 0)
        return -1;

    fprintf(cfg->file, "}\n");

    return 0;
}
