#include "avltree.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct my_node
{
  int value;
  struct avl_node node;
};

void
add_my_node (struct my_node *n, struct avl_root *tree)
{
  struct avl_node *parent = NULL;
  struct avl_node **link = &tree->avl_node;

  while (*link != NULL)
    {
      parent = *link;

      if (n->value < avl_entry (parent, struct my_node, node)->value)
        link = &parent->avl_left;
      else
        link = &parent->avl_right;
    }

  avl_link_node (&n->node, parent, link);
  avl_balance_insert (&n->node, tree);
}

int
main ()
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct avl_node *iter, *n;
  struct my_node *myiter, *mynext;
  int i;

  srand (time (NULL));

  for (i = 0; i < 2000; ++i)
    {
      struct my_node *mynode = (struct my_node *)malloc (sizeof (*mynode));
      mynode->value = rand ();
      add_my_node (mynode, &tree);
    }

  avl_for_each_safe (iter, n, &tree)
    {
      struct my_node *entry = avl_entry (iter, struct my_node, node);
      if (entry->value % 2 == 0)
        {
          avl_erase (iter, &tree);
          free (entry);
        }
    }

  avl_for_each (iter, &tree)
    {
      printf ("%d\n", avl_entry (iter, struct my_node, node)->value);
    }

  avl_for_each_entry (myiter, &tree, node)
    {
      printf ("%d\n", myiter->value);
    }

  avl_for_each_entry_safe (myiter, mynext, &tree, node)
    {
      myiter = NULL;
    }

  avl_for_each_safe (iter, n, &tree)
    {
      struct my_node *entry = avl_entry (iter, struct my_node, node);

      avl_erase (iter, &tree);
      free (entry);
    }

  return 0;
}
