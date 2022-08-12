#include "avltree.c"
#include "rbtree.c"
#include <type_traits>

#ifdef ENABLE_BOOST
#include <boost/intrusive/rbtree.hpp>
#endif

#include <inttypes.h>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die(const char *msg)
{
    perror(msg);
    exit(1);
}

#if defined(_WIN32)
#include <Windows.h>
#define get_time GetTickCount
#else
#include <time.h>
static uint64_t get_time(void)
{
    timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        die("clock_gettime");

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
#endif

static void prepare_sample(size_t nr_samples, size_t block_size,
                           ptrdiff_t value_offset, void *samples)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    for (size_t i = 0; i < nr_samples; ++i) {
        char *block = (char *)samples + block_size * i;
        void *value = block + value_offset;
        int tmp = gen();
        *(int volatile *)value = tmp;
    }
}

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
    int value;
    int dummy[10];
    struct rb_node node;
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
    struct entry_rb *entries = new entry_rb[nr_entries];
    if (!entries)
        die("calloc");

    prepare_sample(nr_entries, sizeof(struct entry_rb),
                   offsetof(struct entry_rb, value), entries);

    uint64_t t0 = get_time();

    for (size_t i = 0; i < nr_entries; ++i) {
        struct entry_rb *entry = &entries[i];
        insert_rb(entry, &tree);
    }

    uint64_t t1 = get_time();

    rb_for_each_safe_init (iter, &tree) {
        rb_erase(iter, &tree);
    }

    uint64_t t2 = get_time();

    delete[] entries;

    print_result("rbtree", nr_entries, t0, t1, t2);
}

struct entry_avl {
    int value;
    int dummy[10];
    struct avl_node node;
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
    struct entry_avl *entries = new entry_avl[nr_entries];
    if (!entries)
        die("calloc");

    prepare_sample(nr_entries, sizeof(struct entry_avl),
                   offsetof(struct entry_avl, value), entries);

    uint64_t t0 = get_time();

    for (size_t i = 0; i < nr_entries; ++i) {
        struct entry_avl *entry = &entries[i];
        insert_avl(entry, &tree);
    }

    uint64_t t1 = get_time();

    avl_for_each_safe_init (iter, &tree) {
        avl_erase(iter, &tree);
    }
    uint64_t t2 = get_time();

    delete[] entries;

    print_result("avltree", nr_entries, t0, t1, t2);
}

#ifdef ENABLE_BOOST
struct entry_irb : boost::intrusive::set_base_hook<> {
    int value;
    int dummy[10];
};

struct key_of_value_irb {
    using type = int;

    int operator()(entry_irb const &e) const noexcept { return e.value; }
};

static void benchmark_irb(size_t nr_entries)
{
    boost::intrusive::rbtree<entry_irb,
                             boost::intrusive::key_of_value<key_of_value_irb>>
        tree;

    struct entry_irb *entries = new entry_irb[nr_entries];

    prepare_sample(nr_entries, sizeof(entry_irb), offsetof(entry_irb, value),
                   entries);

    long t0 = get_time();

    for (size_t i = 0; i < nr_entries; ++i) {
        tree.insert_equal(entries[i]);
    }

    long t1 = get_time();

    tree.clear();

    long t2 = get_time();

    delete[] entries;
    print_result("boost-rbtree", nr_entries, t0, t1, t2);
}

#endif

struct benchmark {
    const char *name;
    void (*proc)(size_t);
};

static const struct benchmark benchmarks[] = {
    {"rbtree", benchmark_rb},
    {"avltree", benchmark_avl},
#ifdef ENABLE_BOOST
    {"boost-rbtree", benchmark_irb},
#endif
    {},
};

static void usage(FILE *out, const char *program)
{
    fprintf(out, "%s ", program);
    bool need_sep = false;
    for (const struct benchmark *iter = benchmarks; iter->name != NULL;
         ++iter) {
        if (need_sep)
            fprintf(out, "|");
        need_sep = true;
        fprintf(out, "%s", iter->name);
    }
    fprintf(out, "\n");
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

    for (const struct benchmark *iter = benchmarks; iter->name != NULL;
         ++iter) {

        if (strcmp(argv[1], iter->name) == 0) {
            iter->proc(atoi(argv[2]));
            return 0;
        }
    }

    usage(stderr, argv[0]);
    return 1;
}
