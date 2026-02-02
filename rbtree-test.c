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

#include "rbtree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct test_node
{
  int value;
  struct rb_node node;
};

static int test_count = 0;
static int pass_count = 0;

#define TEST(name) static void test_##name (void)
#define RUN_TEST(name)                                                        \
  do                                                                          \
    {                                                                         \
      fprintf (stderr, "Running %s... ", #name);                              \
      fflush (stderr);                                                        \
      test_count++;                                                           \
      test_##name ();                                                         \
      fprintf (stderr, "PASS\n");                                             \
      pass_count++;                                                           \
    }                                                                         \
  while (0)

#define ASSERT(cond)                                                          \
  do                                                                          \
    {                                                                         \
      if (!(cond))                                                            \
        {                                                                     \
          fprintf (stderr, "\n  ASSERTION FAILED: %s at line %d\n", #cond,    \
                   __LINE__);                                                 \
          fflush (stdout);                                                    \
          fflush (stderr);                                                    \
          abort ();                                                           \
        }                                                                     \
    }                                                                         \
  while (0)

#define FAIL(msg)                                                             \
  do                                                                          \
    {                                                                         \
      fprintf (stderr, "\n  FAILED: %s at line %d\n", msg, __LINE__);        \
      fflush (stdout);                                                        \
      fflush (stderr);                                                        \
      abort ();                                                               \
    }                                                                         \
  while (0)

/* Validation: Check RB-tree properties */
static int
rb_black_height (const struct rb_node *node)
{
  int left_bh, right_bh;

  if (node == NULL)
    return 1;

  left_bh = rb_black_height (node->rb_left);
  right_bh = rb_black_height (node->rb_right);

  if (left_bh != right_bh)
    FAIL ("Black height mismatch");

  if (!node->rb_is_black)
    {
      if (node->rb_left && !node->rb_left->rb_is_black)
        FAIL ("Red node has red left child");
      if (node->rb_right && !node->rb_right->rb_is_black)
        FAIL ("Red node has red right child");
    }

  return left_bh + (node->rb_is_black ? 1 : 0);
}

static void
rb_validate (struct rb_root *tree)
{
  if (tree->rb_node)
    {
      if (!tree->rb_node->rb_is_black)
        FAIL ("Root is not black");
      rb_black_height (tree->rb_node);
    }
}

static int
rb_count_nodes (const struct rb_node *node)
{
  if (node == NULL)
    return 0;
  return 1 + rb_count_nodes (node->rb_left) + rb_count_nodes (node->rb_right);
}

/* Comparison function for rb_add */
static bool
node_less (const struct rb_node *a, const struct rb_node *b)
{
  const struct test_node *na = rb_entry (a, struct test_node, node);
  const struct test_node *nb = rb_entry (b, struct test_node, node);
  return na->value < nb->value;
}

/* Search comparison */
static int
comp_value (const void *key, const struct rb_node *node)
{
  int val = *(const int *) key;
  const struct test_node *n = rb_entry (node, struct test_node, node);
  if (val < n->value)
    return -1;
  if (val > n->value)
    return 1;
  return 0;
}

/* Insert a value into tree */
static void
insert_value (struct rb_root *tree, int value)
{
  struct test_node *n = malloc (sizeof (*n));
  ASSERT (n != NULL);
  n->value = value;
  rb_add (&n->node, tree, node_less);
}

/* Find a value in tree */
static struct test_node *
find_value (struct rb_root *tree, int value)
{
  struct rb_node *node = rb_find (&value, tree, comp_value);
  return node ? rb_entry (node, struct test_node, node) : NULL;
}

/* Delete a value from tree */
static void
delete_value (struct rb_root *tree, int value)
{
  struct test_node *n = find_value (tree, value);
  if (n)
    {
      rb_erase (&n->node, tree);
      free (n);
    }
}

/* ========== TESTS ========== */

TEST (empty_tree)
{
  struct rb_root tree = RB_ROOT_INIT;
  ASSERT (rb_empty (&tree));
  ASSERT (rb_first (&tree) == NULL);
  ASSERT (rb_last (&tree) == NULL);
  rb_validate (&tree);
}

TEST (single_node)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct test_node n = {.value = 42};

  rb_link_node (&n.node, NULL, &tree.rb_node);
  n.node.rb_is_black = true;

  ASSERT (!rb_empty (&tree));
  ASSERT (rb_first (&tree) == &n.node);
  ASSERT (rb_last (&tree) == &n.node);
  ASSERT (rb_next (&n.node) == NULL);
  ASSERT (rb_prev (&n.node) == NULL);
  rb_validate (&tree);
}

TEST (insert_ascending)
{
  struct rb_root tree = RB_ROOT_INIT;
  int i;

  for (i = 0; i < 100; i++)
    {
      insert_value (&tree, i);
      rb_validate (&tree);
    }

  int count = rb_count_nodes (tree.rb_node);
  printf("DEBUG: Node count after inserts: %d (expected 100)\n", count);
  ASSERT (count == 100);

  for (i = 0; i < 100; i++)
    {
      struct test_node *n = find_value (&tree, i);
      ASSERT (n != NULL);
      ASSERT (n->value == i);
    }
}

TEST (insert_descending)
{
  struct rb_root tree = RB_ROOT_INIT;
  int i;

  for (i = 99; i >= 0; i--)
    {
      insert_value (&tree, i);
      rb_validate (&tree);
    }

  ASSERT (rb_count_nodes (tree.rb_node) == 100);
}

TEST (insert_random)
{
  struct rb_root tree = RB_ROOT_INIT;
  int i;
  srand (42); /* Fixed seed for reproducibility */

  for (i = 0; i < 1000; i++)
    {
      insert_value (&tree, rand () % 10000);
      rb_validate (&tree);
    }
}

TEST (insert_duplicates)
{
  struct rb_root tree = RB_ROOT_INIT;
  int i;

  /* Insert same value multiple times - should go to right subtree */
  for (i = 0; i < 10; i++)
    {
      insert_value (&tree, 5);
      rb_validate (&tree);
    }

  /* Count nodes with value 5 */
  int count = 0;
  struct rb_node *node;
  rb_for_each (node, &tree)
  {
    struct test_node *n = rb_entry (node, struct test_node, node);
    if (n->value == 5)
      count++;
  }
  ASSERT (count == 10);
}

TEST (delete_leaf)
{
  struct rb_root tree = RB_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);

  delete_value (&tree, 5);
  rb_validate (&tree);
  ASSERT (find_value (&tree, 5) == NULL);
  ASSERT (find_value (&tree, 10) != NULL);
  ASSERT (find_value (&tree, 15) != NULL);
}

TEST (delete_one_child)
{
  struct rb_root tree = RB_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);
  insert_value (&tree, 12);

  delete_value (&tree, 15);
  rb_validate (&tree);
  ASSERT (find_value (&tree, 15) == NULL);
  ASSERT (find_value (&tree, 12) != NULL);
}

TEST (delete_two_children)
{
  struct rb_root tree = RB_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);
  insert_value (&tree, 12);
  insert_value (&tree, 20);

  delete_value (&tree, 15);
  rb_validate (&tree);
  ASSERT (find_value (&tree, 15) == NULL);
  ASSERT (find_value (&tree, 10) != NULL);
  ASSERT (find_value (&tree, 20) != NULL);
}

TEST (delete_root)
{
  struct rb_root tree = RB_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 5);
  insert_value (&tree, 15);

  delete_value (&tree, 10);
  rb_validate (&tree);
  ASSERT (find_value (&tree, 10) == NULL);
  ASSERT (!rb_empty (&tree));
}

TEST (delete_all)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct rb_node *node, *tmp;
  int i;

  srand (123);
  for (i = 0; i < 100; i++)
    insert_value (&tree, rand () % 200);

  /* Delete all nodes */
  rb_for_each_safe (node, tmp, &tree)
  {
    struct test_node *n = rb_entry (node, struct test_node, node);
    rb_erase (node, &tree);
    free (n);
    rb_validate (&tree);
  }

  ASSERT (rb_empty (&tree));
}

TEST (iteration)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct rb_node *node;
  int values[] = {5, 3, 7, 1, 4, 6, 8};
  int expected[] = {1, 3, 4, 5, 6, 7, 8};
  int i, idx = 0;

  for (i = 0; i < 7; i++)
    insert_value (&tree, values[i]);

  rb_for_each (node, &tree)
  {
    struct test_node *n = rb_entry (node, struct test_node, node);
    ASSERT (n->value == expected[idx]);
    idx++;
  }
  ASSERT (idx == 7);
}

TEST (reverse_iteration)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct rb_node *node;
  int i;

  for (i = 0; i < 10; i++)
    insert_value (&tree, i);

  node = rb_last (&tree);
  i = 9;
  while (node)
    {
      struct test_node *n = rb_entry (node, struct test_node, node);
      ASSERT (n->value == i);
      i--;
      node = rb_prev (node);
    }
  ASSERT (i == -1);
}

TEST (safe_iteration_with_delete)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct rb_node *node, *tmp;
  struct test_node *n;
  int i;

  for (i = 0; i < 100; i++)
    insert_value (&tree, i);

  /* Delete even values during iteration */
  rb_for_each_safe (node, tmp, &tree)
  {
    n = rb_entry (node, struct test_node, node);
    if (n->value % 2 == 0)
      {
        rb_erase (node, &tree);
        free (n);
      }
  }

  rb_validate (&tree);

  /* Verify only odd values remain */
  i = 1;
  rb_for_each (node, &tree)
  {
    n = rb_entry (node, struct test_node, node);
    ASSERT (n->value == i);
    i += 2;
  }
}

TEST (entry_iteration)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct test_node *pos;
  int sum = 0;
  int i;

  for (i = 1; i <= 10; i++)
    insert_value (&tree, i);

  rb_for_each_entry (pos, &tree, node)
  {
    sum += pos->value;
  }

  ASSERT (sum == 55); /* 1+2+...+10 */
}

TEST (entry_safe_iteration)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct test_node *pos, *tmp;
  int count = 0;
  int i;

  for (i = 0; i < 50; i++)
    insert_value (&tree, i);

  rb_for_each_entry_safe (pos, tmp, &tree, node)
  {
    if (pos->value >= 25)
      {
        rb_erase (&pos->node, &tree);
        free (pos);
      }
    count++;
  }

  ASSERT (count == 50);
  ASSERT (rb_count_nodes (tree.rb_node) == 25);
}

TEST (find_nonexistent)
{
  struct rb_root tree = RB_ROOT_INIT;
  insert_value (&tree, 10);
  insert_value (&tree, 20);

  ASSERT (find_value (&tree, 5) == NULL);
  ASSERT (find_value (&tree, 15) == NULL);
  ASSERT (find_value (&tree, 25) == NULL);
}

TEST (first_last)
{
  struct rb_root tree = RB_ROOT_INIT;
  struct test_node *n;

  insert_value (&tree, 50);
  insert_value (&tree, 30);
  insert_value (&tree, 70);
  insert_value (&tree, 20);
  insert_value (&tree, 80);

  n = rb_entry_safe (rb_first (&tree), struct test_node, node);
  ASSERT (n != NULL && n->value == 20);

  n = rb_entry_safe (rb_last (&tree), struct test_node, node);
  ASSERT (n != NULL && n->value == 80);
}

TEST (stress_random)
{
  struct rb_root tree = RB_ROOT_INIT;
  int *values;
  int n_values = 10000;
  int i;

  values = malloc (n_values * sizeof (int));
  ASSERT (values != NULL);

  srand (time (NULL));

  /* Insert random values */
  for (i = 0; i < n_values; i++)
    {
      values[i] = rand ();
      insert_value (&tree, values[i]);
    }

  rb_validate (&tree);
  ASSERT (rb_count_nodes (tree.rb_node) == n_values);

  /* Shuffle values array */
  for (i = n_values - 1; i > 0; i--)
    {
      int j = rand () % (i + 1);
      int tmp_val = values[i];
      values[i] = values[j];
      values[j] = tmp_val;
    }

  /* Delete in random order */
  for (i = 0; i < n_values; i++)
    {
      delete_value (&tree, values[i]);
      rb_validate (&tree);
    }

  ASSERT (rb_empty (&tree));
  free (values);
}

TEST (tree_height)
{
  struct rb_root tree = RB_ROOT_INIT;
  int i;

  /* Insert 1000 sequential values - height should be O(log n) */
  for (i = 0; i < 1000; i++)
    insert_value (&tree, i);

  int bh = rb_black_height (tree.rb_node);
  /* Black height should be <= log2(n) + 1 */
  ASSERT (bh <= 12); /* log2(1000) ≈ 10 */
}

int
main (void)
{
  fprintf (stderr, "=== RB-Tree Comprehensive Test Suite ===\n\n");

  RUN_TEST (empty_tree);
  RUN_TEST (single_node);
  RUN_TEST (insert_ascending);
  RUN_TEST (insert_descending);
  RUN_TEST (insert_random);
  RUN_TEST (insert_duplicates);
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
  RUN_TEST (find_nonexistent);
  RUN_TEST (first_last);
  RUN_TEST (stress_random);
  RUN_TEST (tree_height);

  fprintf (stderr, "\n=== Results ===\n");
  fprintf (stderr, "Passed: %d/%d\n", pass_count, test_count);

  return (pass_count == test_count) ? 0 : 1;
}
