/* Copyright © 2026  Zhengyi Fu <i@fuzy.me> */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "avltree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct test_node
{
  int value;
  struct avl_node node;
};

static int test_count = 0;
static int pass_count = 0;

#define TEST(name) static void test_##name (void)
#define RUN_TEST(name)                                                        \
  do                                                                          \
    {                                                                         \
      printf ("Running %s... ", #name);                                       \
      test_count++;                                                           \
      test_##name ();                                                         \
      printf ("PASS\n");                                                      \
      pass_count++;                                                           \
    }                                                                         \
  while (0)

#define ASSERT(cond)                                                          \
  do                                                                          \
    {                                                                         \
      if (!(cond))                                                            \
        {                                                                     \
          printf ("\n  ASSERTION FAILED: %s at line %d\n", #cond, __LINE__);  \
          abort ();                                                           \
        }                                                                     \
    }                                                                         \
  while (0)

#define FAIL(msg)                                                             \
  do                                                                          \
    {                                                                         \
      printf ("\n  FAILED: %s at line %d\n", msg, __LINE__);                  \
      abort ();                                                               \
    }                                                                         \
  while (0)

/* Validation: Check AVL-tree properties */
static int
avl_height (const struct avl_node *node)
{
  int left_h, right_h;

  if (node == NULL)
    return 0;

  left_h = avl_height (node->avl_left);
  right_h = avl_height (node->avl_right);

  int balance = right_h - left_h;
  if (balance != node->avl_balance)
    FAIL ("Stored balance factor doesn't match actual height difference");

  if (balance < -1 || balance > 1)
    FAIL ("Balance factor out of range (-1, 0, 1)");

  return 1 + (left_h > right_h ? left_h : right_h);
}

static void
avl_validate (struct avl_root *tree)
{
  if (tree->avl_node)
    {
      avl_height (tree->avl_node);
    }
}

static int
avl_count_nodes (const struct avl_node *node)
{
  if (node == NULL)
    return 0;
  return 1 + avl_count_nodes (node->avl_left) + avl_count_nodes (node->avl_right);
}

/* Comparison for insertion */
static int
comp_value (const void *key, struct avl_node *node)
{
  int val = *(const int *) key;
  struct test_node *n = avl_entry (node, struct test_node, node);
  if (val < n->value)
    return -1;
  if (val > n->value)
    return 1;
  return 0;
}

/* Insert a value into tree */
static void
insert_value (struct avl_root *tree, int value)
{
  struct avl_node *parent = NULL;
  struct avl_node **link = &tree->avl_node;

  while (*link != NULL)
    {
      parent = *link;
      if (value < avl_entry (parent, struct test_node, node)->value)
        link = &parent->avl_left;
      else
        link = &parent->avl_right;
    }

  struct test_node *n = malloc (sizeof (*n));
  ASSERT (n != NULL);
  n->value = value;
  avl_link_node (&n->node, parent, link);
  avl_balance_insert (&n->node, tree);
}

/* Find a value in tree */
static struct test_node *
find_value (struct avl_root *tree, int value)
{
  struct avl_node *node = tree->avl_node;

  while (node != NULL)
    {
      int c = comp_value (&value, node);
      if (c < 0)
        node = node->avl_left;
      else if (c > 0)
        node = node->avl_right;
      else
        return avl_entry (node, struct test_node, node);
    }
  return NULL;
}

/* Delete a value from tree */
static void
delete_value (struct avl_root *tree, int value)
{
  struct test_node *n = find_value (tree, value);
  if (n)
    {
      avl_erase (&n->node, tree);
      free (n);
    }
}

/* ========== TESTS ========== */

TEST (empty_tree)
{
  struct avl_root tree = AVL_ROOT_INIT;
  ASSERT (avl_empty (&tree));
  ASSERT (avl_first (&tree) == NULL);
  ASSERT (avl_last (&tree) == NULL);
  avl_validate (&tree);
}

TEST (single_node)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct test_node n = {.value = 42};

  avl_link_node (&n.node, NULL, &tree.avl_node);
  n.node.avl_balance = 0;

  ASSERT (!avl_empty (&tree));
  ASSERT (avl_first (&tree) == &n.node);
  ASSERT (avl_last (&tree) == &n.node);
  ASSERT (avl_next (&n.node) == NULL);
  ASSERT (avl_prev (&n.node) == NULL);
  avl_validate (&tree);
}

TEST (insert_left_rotation)
{
  /* Triggers left rotation: insert 1, 2, 3 */
  struct avl_root tree = AVL_ROOT_INIT;

  insert_value (&tree, 1);
  avl_validate (&tree);

  insert_value (&tree, 2);
  avl_validate (&tree);

  insert_value (&tree, 3);
  avl_validate (&tree);

  ASSERT (avl_count_nodes (tree.avl_node) == 3);
  ASSERT (find_value (&tree, 2) != NULL); /* 2 should be root after rotation */
}

TEST (insert_right_rotation)
{
  /* Triggers right rotation: insert 3, 2, 1 */
  struct avl_root tree = AVL_ROOT_INIT;

  insert_value (&tree, 3);
  avl_validate (&tree);

  insert_value (&tree, 2);
  avl_validate (&tree);

  insert_value (&tree, 1);
  avl_validate (&tree);

  ASSERT (avl_count_nodes (tree.avl_node) == 3);
}

TEST (insert_left_right_rotation)
{
  /* Triggers left-right rotation: insert 3, 1, 2 */
  struct avl_root tree = AVL_ROOT_INIT;

  insert_value (&tree, 3);
  insert_value (&tree, 1);
  insert_value (&tree, 2);

  avl_validate (&tree);
  ASSERT (avl_count_nodes (tree.avl_node) == 3);
  ASSERT (find_value (&tree, 2) != NULL);
}

TEST (insert_right_left_rotation)
{
  /* Triggers right-left rotation: insert 1, 3, 2 */
  struct avl_root tree = AVL_ROOT_INIT;

  insert_value (&tree, 1);
  insert_value (&tree, 3);
  insert_value (&tree, 2);

  avl_validate (&tree);
  ASSERT (avl_count_nodes (tree.avl_node) == 3);
}

TEST (insert_ascending)
{
  struct avl_root tree = AVL_ROOT_INIT;
  int i;

  for (i = 0; i < 100; i++)
    {
      insert_value (&tree, i);
      avl_validate (&tree);
    }

  ASSERT (avl_count_nodes (tree.avl_node) == 100);

  for (i = 0; i < 100; i++)
    {
      struct test_node *n = find_value (&tree, i);
      ASSERT (n != NULL);
      ASSERT (n->value == i);
    }
}

TEST (insert_descending)
{
  struct avl_root tree = AVL_ROOT_INIT;
  int i;

  for (i = 99; i >= 0; i--)
    {
      insert_value (&tree, i);
      avl_validate (&tree);
    }

  ASSERT (avl_count_nodes (tree.avl_node) == 100);
}

TEST (insert_random)
{
  struct avl_root tree = AVL_ROOT_INIT;
  int i;
  srand (42);

  for (i = 0; i < 1000; i++)
    {
      insert_value (&tree, rand () % 10000);
      avl_validate (&tree);
    }
}

TEST (delete_leaf)
{
  struct avl_root tree = AVL_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);

  delete_value (&tree, 5);
  avl_validate (&tree);
  ASSERT (find_value (&tree, 5) == NULL);
  ASSERT (find_value (&tree, 10) != NULL);
  ASSERT (find_value (&tree, 15) != NULL);
}

TEST (delete_one_child)
{
  struct avl_root tree = AVL_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);
  insert_value (&tree, 12);

  delete_value (&tree, 15);
  avl_validate (&tree);
  ASSERT (find_value (&tree, 15) == NULL);
}

TEST (delete_two_children)
{
  struct avl_root tree = AVL_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);
  insert_value (&tree, 12);
  insert_value (&tree, 20);

  delete_value (&tree, 15);
  avl_validate (&tree);
  ASSERT (find_value (&tree, 15) == NULL);
}

TEST (delete_root)
{
  struct avl_root tree = AVL_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);

  delete_value (&tree, 10);
  avl_validate (&tree);
  ASSERT (find_value (&tree, 10) == NULL);
  ASSERT (!avl_empty (&tree));
}

TEST (delete_all)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct avl_node *node, *tmp;
  int i;

  srand (123);
  for (i = 0; i < 100; i++)
    insert_value (&tree, rand () % 200);

  avl_for_each_safe (node, tmp, &tree)
  {
    struct test_node *n = avl_entry (node, struct test_node, node);
    avl_erase (node, &tree);
    free (n);
    avl_validate (&tree);
  }

  ASSERT (avl_empty (&tree));
}

TEST (iteration)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct avl_node *node;
  int values[] = {5, 3, 7, 1, 4, 6, 8};
  int expected[] = {1, 3, 4, 5, 6, 7, 8};
  int i, idx = 0;

  for (i = 0; i < 7; i++)
    insert_value (&tree, values[i]);

  avl_for_each (node, &tree)
  {
    struct test_node *n = avl_entry (node, struct test_node, node);
    ASSERT (n->value == expected[idx]);
    idx++;
  }
  ASSERT (idx == 7);
}

TEST (reverse_iteration)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct avl_node *node;
  int i;

  for (i = 0; i < 10; i++)
    insert_value (&tree, i);

  node = avl_last (&tree);
  i = 9;
  while (node)
    {
      struct test_node *n = avl_entry (node, struct test_node, node);
      ASSERT (n->value == i);
      i--;
      node = avl_prev (node);
    }
  ASSERT (i == -1);
}

TEST (safe_iteration_with_delete)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct avl_node *node, *tmp;
  struct test_node *n;
  int i;

  for (i = 0; i < 100; i++)
    insert_value (&tree, i);

  avl_for_each_safe (node, tmp, &tree)
  {
    n = avl_entry (node, struct test_node, node);
    if (n->value % 2 == 0)
      {
        avl_erase (node, &tree);
        free (n);
      }
  }

  avl_validate (&tree);

  i = 1;
  avl_for_each (node, &tree)
  {
    n = avl_entry (node, struct test_node, node);
    ASSERT (n->value == i);
    i += 2;
  }
}

TEST (entry_iteration)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct test_node *pos;
  int sum = 0;
  int i;

  for (i = 1; i <= 10; i++)
    insert_value (&tree, i);

  avl_for_each_entry (pos, &tree, node)
  {
    sum += pos->value;
  }

  ASSERT (sum == 55);
}

TEST (entry_safe_iteration)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct test_node *pos, *tmp;
  int count = 0;
  int i;

  for (i = 0; i < 50; i++)
    insert_value (&tree, i);

  avl_for_each_entry_safe (pos, tmp, &tree, node)
  {
    if (pos->value >= 25)
      {
        avl_erase (&pos->node, &tree);
        free (pos);
      }
    count++;
  }

  ASSERT (count == 50);
  ASSERT (avl_count_nodes (tree.avl_node) == 25);
}

TEST (first_last)
{
  struct avl_root tree = AVL_ROOT_INIT;
  struct test_node *n;

  insert_value (&tree, 50);
  insert_value (&tree, 30);
  insert_value (&tree, 70);
  insert_value (&tree, 20);
  insert_value (&tree, 80);

  n = avl_entry_safe (avl_first (&tree), struct test_node, node);
  ASSERT (n != NULL && n->value == 20);

  n = avl_entry_safe (avl_last (&tree), struct test_node, node);
  ASSERT (n != NULL && n->value == 80);
}

TEST (stress_random)
{
  struct avl_root tree = AVL_ROOT_INIT;
  int *values;
  int n_values = 10000;
  int i;

  values = malloc (n_values * sizeof (int));
  ASSERT (values != NULL);

  srand (time (NULL));

  for (i = 0; i < n_values; i++)
    {
      values[i] = rand ();
      insert_value (&tree, values[i]);
    }

  avl_validate (&tree);
  ASSERT (avl_count_nodes (tree.avl_node) == n_values);

  for (i = n_values - 1; i > 0; i--)
    {
      int j = rand () % (i + 1);
      int tmp_val = values[i];
      values[i] = values[j];
      values[j] = tmp_val;
    }

  for (i = 0; i < n_values; i++)
    {
      delete_value (&tree, values[i]);
      avl_validate (&tree);
    }

  ASSERT (avl_empty (&tree));
  free (values);
}

TEST (tree_balance)
{
  struct avl_root tree = AVL_ROOT_INIT;
  int i;

  for (i = 0; i < 1000; i++)
    insert_value (&tree, i);

  int h = avl_height (tree.avl_node);
  /* AVL tree height <= 1.44 * log2(n+2) - 0.328 */
  ASSERT (h <= 15); /* For n=1000, max height ~14.4 */
}

TEST (alternating_insert_delete)
{
  struct avl_root tree = AVL_ROOT_INIT;
  int i;

  for (i = 0; i < 100; i++)
    {
      insert_value (&tree, i);
      avl_validate (&tree);
    }

  for (i = 0; i < 50; i++)
    {
      delete_value (&tree, i * 2);
      avl_validate (&tree);
    }

  ASSERT (avl_count_nodes (tree.avl_node) == 50);
}

int
main (void)
{
  printf ("=== AVL Tree Comprehensive Test Suite ===\n\n");

  RUN_TEST (empty_tree);
  RUN_TEST (single_node);
  RUN_TEST (insert_left_rotation);
  RUN_TEST (insert_right_rotation);
  RUN_TEST (insert_left_right_rotation);
  RUN_TEST (insert_right_left_rotation);
  RUN_TEST (insert_ascending);
  RUN_TEST (insert_descending);
  RUN_TEST (insert_random);
  RUN_TEST (delete_leaf);
  RUN_TEST (delete_one_child);
  RUN_TEST (delete_two_children);
  RUN_TEST (delete_root);
  RUN_TEST (delete_all);
  RUN_TEST (iteration);
  RUN_TEST (reverse_iteration);
  RUN_TEST (safe_iteration_with_delete);
  RUN_TEST (entry_iteration);
  RUN_TEST (entry_safe_iteration);
  RUN_TEST (first_last);
  RUN_TEST (stress_random);
  RUN_TEST (tree_balance);
  RUN_TEST (alternating_insert_delete);

  printf ("\n=== Results ===\n");
  printf ("Passed: %d/%d\n", pass_count, test_count);

  return (pass_count == test_count) ? 0 : 1;
}
