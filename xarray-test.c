#include "xarray.h"
#include <stdio.h>
#include <limits.h>

int main()
{
    unsigned long i;
    struct xarray xa = XA_INIT;
    // void *v;


    for (i = 0; i < 100; ++i) {
        if (xa_store(&xa, i*10, &i)) {
            printf("failed to insert %li\n", i*10);
        }
    }

    for (i = 0; i < 1000; ++i) {
        if (xa_load(&xa, i)) {
            printf("set %li\n", i);
        }
    }

    i = 0;
    if (xa_store(&xa, i, &i)) {
        perror("failed to insert 0");
    }

    i = 0xffffffff;
    if (xa_store(&xa, i, &i)) {
        printf("failed to insert %li\n", i*1000);
    }
    printf("size: %lu\n", xa_size(&xa));
    printf("levels: %u\n", xa.xa_levels);
    printf("num_nodes: %lu\n",xa.xa_node_num);

    xa_erase(&xa, 0);
    xa_release(&xa);

    printf("size: %lu\n", xa_size(&xa));
    printf("levels: %u\n", xa.xa_levels);
    printf("num_nodes: %lu\n",xa.xa_node_num);

    // xa_for_each (&xa, i, v) {
    //     printf("xa[%li] = %p\n", i, v);
    // }

    xa_destroy(&xa);

    printf("success\n");
}
