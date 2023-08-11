#include "xarray.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

int
main ()
{
  unsigned long i;
  struct xarray xa = XA_INIT;
  void *v;

  for (i = 0; i < 100; ++i)
    {
      if (xa_store (&xa, i * 10, &i) == XA_FAILED)
        {
          printf ("failed to insert %li\n", i * 10);
        }
    }

  xa_for_each (&xa, i, v)
    {
      assert (v != 0);
      assert (i <= 1000);
      assert (xa_load (&xa, i) == v);
      printf ("%lu = %p\n", i, v);
    }

  i = 0;
  if (xa_store (&xa, i, &i) == XA_FAILED)
    {
      perror ("failed to insert 0");
    }

  i = 0xffffffff;
  if (xa_store (&xa, i, &i) == XA_FAILED)
    {
      printf ("failed to insert %li\n", i * 1000);
    }
  printf ("size: %lu\n", xa_size (&xa));
  printf ("levels: %u\n", xa.xa_levels);
  printf ("num_nodes: %lu\n", xa.xa_node_num);

  xa_erase (&xa, i);
  xa_release (&xa);

  printf ("size: %lu\n", xa_size (&xa));
  printf ("levels: %u\n", xa.xa_levels);
  printf ("num_nodes: %lu\n", xa.xa_node_num);

  xa_destroy (&xa);

  printf ("success\n");

  return 0;
}
