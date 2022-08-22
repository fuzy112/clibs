#include "avltree.c"
#include "rbtree.c"
#include "splay.c"
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

typedef void (*prepare_sample_func_t)(size_t nr_samples, size_t block_size,
                                      ptrdiff_t value_offset, void *samples);

static void prepare_sample_random(size_t nr_samples, size_t block_size,
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

static void prepare_sample_monotonic(size_t nr_samples, size_t block_size,
                                     ptrdiff_t value_offset, void *samples)
{
    for (size_t i = 0; i < nr_samples; ++i) {
        char *block = (char *)samples + block_size * i;
        void *value = block + value_offset;
        int tmp = i;
        *(int volatile *)value = tmp;
    }
}

static void prepare_sample_normal_distribution(size_t nr_samples,
                                               size_t block_size,
                                               ptrdiff_t value_offset,
                                               void *samples)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> norm(0, 0x1000);
    for (size_t i = 0; i < nr_samples; ++i) {
        char *block = (char *)samples + block_size * i;
        void *value = block + value_offset;
        int tmp = norm(gen);
        *(int volatile *)value = tmp;
    }
}

struct sample_func {
    const char *name;
    prepare_sample_func_t func;
};

static struct sample_func sample_funcs[] = {
    {"random", prepare_sample_random},
    {"monotonic", prepare_sample_monotonic},
    {"normal", prepare_sample_normal_distribution},
    {},
};

static void print_result(const char *name, size_t nr_entries, uint64_t t0,
                         uint64_t t1, uint64_t t2)
{
    printf("name:       %16s\n", name);
    printf("samples:    %16lu\n", (unsigned long)nr_entries);
    printf("insert:     %16" PRIu64 "\n", t1 - t0);
    printf("search:     %16" PRIu64 "\n", t2 - t1);
    printf("total:      %16" PRIu64 "\n", t2 - t0);
}

void *volatile search_result;

struct entry_rb {
    int value;
    int dummy[10];
    struct rb_node node;
};

static void insert_rb(struct entry_rb *entry, struct rb_root *tree)
{
    struct rb_node *parent = NULL;
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

static struct entry_rb *search_rb(int value, const struct rb_root *tree)
{
    struct rb_node *parent = tree->rb_node;

    while (parent) {
        int c = value - rb_entry(parent, struct entry_rb, node)->value;
        if (c < 0)
            parent = parent->rb_left;
        else if (c > 0)
            parent = parent->rb_right;
        else
            return rb_entry(parent, struct entry_rb, node);
        ;
    }

    return NULL;
}

static void benchmark_rb(size_t nr_entries,
                         prepare_sample_func_t prepare_sample)
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

    for (size_t i = 0; i < nr_entries; ++i) {
        for (int j = 0; j < 5; ++j)
            search_result = search_rb(entries[i].value, &tree);
    }

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

static struct entry_avl *search_avl(int value, const struct avl_root *tree)
{
    struct avl_node *parent = tree->avl_node;

    while (parent) {
        int c = value - avl_entry(parent, struct entry_avl, node)->value;
        if (c < 0)
            parent = parent->avl_left;
        else if (c > 0)
            parent = parent->avl_right;
        else
            return avl_entry(parent, struct entry_avl, node);
    }

    return NULL;
}

static void benchmark_avl(size_t nr_entries,
                          prepare_sample_func_t prepare_sample)
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

    for (size_t i = 0; i < nr_entries; ++i) {
        for (int j = 0; j < 5; ++j)

            search_result = search_avl(entries[i].value, &tree);
    }

    avl_for_each_safe_init (iter, &tree) {
        avl_erase(iter, &tree);
    }
    uint64_t t2 = get_time();

    delete[] entries;

    print_result("avltree", nr_entries, t0, t1, t2);
}

struct entry_splay {
    int value;
    int dummy[10];
    struct splay_node node;
};

static void insert_splay(struct entry_splay *entry, struct splay_root *tree)
{
    struct splay_node *parent = splay_end(tree);
    struct splay_node **link = &tree->splay_node;

    while (*link) {
        parent = *link;

        if (entry->value < splay_entry(parent, struct entry_splay, node)->value)
            link = &parent->splay_left;
        else
            link = &parent->splay_right;
    }

    splay_link_node(&entry->node, parent, link);
    splay_balance(&entry->node, tree);
}

static struct entry_splay *search_splay(int value, struct splay_root *tree)
{
    struct splay_node *parent = tree->splay_node;

    while (parent) {
        int c = value - splay_entry(parent, struct entry_splay, node)->value;
        if (c < 0)
            parent = parent->splay_left;
        else if (c > 0)
            parent = parent->splay_right;
        else {
            splay_balance(parent, tree);
            return splay_entry(parent, struct entry_splay, node);
        }
    }

    return NULL;
}

static void benchmark_splay(size_t nr_entries,
                            prepare_sample_func_t prepare_sample)
{
    struct splay_root tree = SPLAY_ROOT_INIT;
    struct entry_splay *entries = new entry_splay[nr_entries]();
    if (!entries)
        die("calloc");

    prepare_sample(nr_entries, sizeof(struct entry_splay),
                   offsetof(struct entry_splay, value), entries);

    uint64_t t0 = get_time();

    for (size_t i = 0; i < nr_entries; ++i) {
        struct entry_splay *entry = &entries[i];
        insert_splay(entry, &tree);
    }

    uint64_t t1 = get_time();

    for (size_t i = 0; i < nr_entries; ++i) {
        for (int j = 0; j < 5; ++j)
            search_result = search_splay(entries[i].value, &tree);
    }

    splay_for_each_safe_init (iter, &tree) {
        splay_erase(iter, &tree);
    }
    uint64_t t2 = get_time();

    delete[] entries;

    print_result("splay", nr_entries, t0, t1, t2);
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

static void benchmark_irb(size_t nr_entries,
                          prepare_sample_func_t prepare_sample)
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

    for (size_t i = 0; i < nr_entries; ++i) {
        search_result = &*tree.find(entries[i].value);
    }

    tree.clear();

    long t2 = get_time();

    delete[] entries;
    print_result("boost-rbtree", nr_entries, t0, t1, t2);
}

#endif

struct benchmark {
    const char *name;
    void (*proc)(size_t, prepare_sample_func_t);
};

static const struct benchmark benchmarks[] = {
    {"rbtree", benchmark_rb},
    {"avltree", benchmark_avl},
    {"splay", benchmark_splay},
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
    if (argc < 3) {
        usage(stderr, argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        usage(stdout, argv[0]);
        return 0;
    }

    prepare_sample_func_t prepare_sample = prepare_sample_random;
    if (argc > 3) {
        for (const struct sample_func *iter = sample_funcs; iter->name != NULL;
             ++iter) {
            if (strcmp(argv[3], iter->name) == 0) {
                prepare_sample = iter->func;
                break;
            }
        }
    }

    for (const struct benchmark *iter = benchmarks; iter->name != NULL;
         ++iter) {

        if (strcmp(argv[1], iter->name) == 0) {
            iter->proc(atoi(argv[2]), prepare_sample);
            return 0;
        }
    }

    usage(stderr, argv[0]);
    return 1;
}
