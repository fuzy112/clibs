#include "avltree.h"
#include "rbtree.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void die(const char *msg)
{
    perror(msg);
    exit(1);
}

static void usage(FILE *out, const char *program)
{
    fprintf(out, "%s rbtree|avltree samples\n", program);
}

#if __STDC_VERSION__ >= 201101L
static uint64_t get_time(void)
{
    struct timespec ts;

    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
#elif defined(_WIN32)
#include <Windows.h>
#define get_time GetTickCount
#endif

static void print_result(const char *name, size_t nr_entries, uint64_t t0,
                         uint64_t t1, uint64_t t2)
{
    printf("name:       %16s\n", name);
    printf("samples:    %16lu\n", (unsigned long)nr_entries);
    printf("insertion:  %16" PRIu64 "\n", t1 - t0);
    printf("deletion:   %16" PRIu64 "\n", t2 - t1);
    printf("total:      %16" PRIu64 "\n", t2 - t0);
}

struct entry_rb {
    struct rb_node node;
    int value;
};

static void insert_rb(struct entry_rb *entry, struct rb_root *tree)
{
    struct rb_node *parent = rb_end(tree);
    struct rb_node **link = &tree->rb_node;

    while (*link) {
        parent = *link;

        if (entry->value < rb_entry(parent, struct entry_rb, node)->value)
            link = &parent->rb_left;
        else
            link = &parent->rb_right;
    }

    rb_link_node(&entry->node, parent, link);
    rb_balance_insert(&entry->node, tree);
}

static void benchmark_rb(size_t nr_entries)
{
    struct rb_root tree = RB_ROOT_INIT;
    struct entry_rb *entries = calloc(nr_entries, sizeof(struct entry_rb));
    if (!entries)
        die("calloc");

    uint64_t t0 = get_time();

    for (size_t i = 0; i < nr_entries; ++i) {
        struct entry_rb *entry = &entries[i];
        entry->value = i % 13 + i % 47;
        insert_rb(entry, &tree);
    }

    uint64_t t1 = get_time();

    rb_for_each_entry_safe_init(struct entry_rb, iter, &tree, node)
    {
        rb_erase(&iter->node, &tree);
    }

    uint64_t t2 = get_time();

    free(entries);

    print_result("rbtree", nr_entries, t0, t1, t2);
}

struct entry_avl {
    struct avl_node node;
    int value;
};

static void insert_avl(struct entry_avl *entry, struct avl_root *tree)
{
    struct avl_node *parent = avl_end(tree);
    struct avl_node **link = &tree->avl_node;

    while (*link) {
        parent = *link;

        if (entry->value < avl_entry(parent, struct entry_avl, node)->value)
            link = &parent->avl_left;
        else
            link = &parent->avl_right;
    }

    avl_link_node(&entry->node, parent, link);
    avl_balance_insert(&entry->node, tree);
}

static void benchmark_avl(size_t nr_entries)
{
    struct avl_root tree = AVL_ROOT_INIT;
    struct entry_avl *entries = calloc(nr_entries, sizeof(struct entry_avl));
    if (!entries)
        die("calloc");

    uint64_t t0 = get_time();

    for (size_t i = 0; i < nr_entries; ++i) {
        struct entry_avl *entry = &entries[i];
        entry->value = i % 13 + i % 17;
        insert_avl(entry, &tree);
    }

    uint64_t t1 = get_time();

    avl_for_each_entry_safe_init(struct entry_avl, iter, &tree, node)
    {
        avl_erase(&iter->node, &tree);
    }

    uint64_t t2 = get_time();

    free(entries);

    print_result("avltree", nr_entries, t0, t1, t2);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        usage(stderr, argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        usage(stdout, argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "rbtree") == 0) {
        benchmark_rb(atoi(argv[2]));
    } else if (strcmp(argv[1], "avltree") == 0) {
        benchmark_avl(atoi(argv[2]));
    } else {
        usage(stderr, argv[0]);
        return 1;
    }
    return 0;
}
