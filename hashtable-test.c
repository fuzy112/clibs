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

#include "hashtable.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct test_item
{
  int key;
  int value;
  struct hlist_node node;
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

/* ========== TESTS ========== */

TEST (basic_operations)
{
  HASHTABLE (table, 8);
  struct test_item *item;

  hash_init (table);

  /* Insert items */
  for (int i = 0; i < 10; i++)
    {
      item = malloc (sizeof (*item));
      ASSERT (item != NULL);
      item->key = i;
      item->value = i * 10;
      hash_add (table, i, &item->node);
    }

  /* Count items */
  int count = 0;
  unsigned bucket;
  hash_for_each_entry (table, bucket, item, node)
  {
    count++;
  }
  ASSERT (count == 10);

  /* Find items */
  for (int i = 0; i < 10; i++)
    {
      struct test_item *found = NULL;
      hash_for_each_possible_entry (table, i, item, node)
      {
        if (item->key == i)
          {
            found = item;
            break;
          }
      }
      ASSERT (found != NULL);
      ASSERT (found->key == i);
      ASSERT (found->value == i * 10);
    }

  /* Clean up */
  struct hlist_node *tmp;
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    hlist_del (&item->node);
    free (item);
  }
}

TEST (collisions)
{
  HASHTABLE (table, 4); /* Small table to force collisions */
  struct test_item *items[20];

  hash_init (table);

  /* Insert items that will collide (same hash bucket) */
  for (int i = 0; i < 20; i++)
    {
      items[i] = malloc (sizeof (*items[i]));
      ASSERT (items[i] != NULL);
      items[i]->key = i;
      items[i]->value = i * 100;
      hash_add (table, i, &items[i]->node);
    }

  /* Count items */
  int count = 0;
  struct test_item *item;
  unsigned bucket;
  hash_for_each_entry (table, bucket, item, node)
  {
    count++;
  }
  ASSERT (count == 20);

  /* Verify all items can be found */
  for (int i = 0; i < 20; i++)
    {
      struct test_item *found = NULL;
      hash_for_each_possible_entry (table, i, item, node)
      {
        if (item->key == i)
          {
            found = item;
            break;
          }
      }
      ASSERT (found != NULL);
      ASSERT (found->value == i * 100);
    }

  /* Clean up */
  struct hlist_node *tmp;
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    hlist_del (&item->node);
    free (item);
  }
}

TEST (delete_from_bucket)
{
  HASHTABLE (table, 4);
  struct test_item *items[10];

  hash_init (table);

  /* Insert items */
  for (int i = 0; i < 10; i++)
    {
      items[i] = malloc (sizeof (*items[i]));
      items[i]->key = i;
      items[i]->value = i;
      hash_add (table, i, &items[i]->node);
    }

  /* Delete middle item from a bucket */
  hlist_del (&items[5]->node);
  free (items[5]);

  /* Verify others still exist */
  int count = 0;
  struct test_item *item;
  unsigned bucket;
  hash_for_each_entry (table, bucket, item, node)
  {
    count++;
  }
  ASSERT (count == 9);

  /* Clean up */
  struct hlist_node *tmp;
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    hlist_del (&item->node);
    free (item);
  }
}

TEST (iterate_all_buckets)
{
  HASHTABLE (table, 8);
  int sum = 0;
  struct test_item *item;
  unsigned bucket;

  hash_init (table);

  /* Insert items to different buckets */
  for (int i = 0; i < 256; i++)
    {
      item = malloc (sizeof (*item));
      item->key = i;
      item->value = i;
      hash_add (table, i, &item->node);
    }

  /* Iterate all buckets and sum values */
  hash_for_each_entry (table, bucket, item, node)
  {
    sum += item->value;
  }

  ASSERT (sum == 255 * 256 / 2); /* Sum of 0 to 255 */

  /* Clean up */
  struct hlist_node *tmp;
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    hlist_del (&item->node);
    free (item);
  }
}

TEST (safe_iteration_with_delete)
{
  HASHTABLE (table, 8);
  struct test_item *item;
  struct hlist_node *tmp;
  unsigned bucket;

  hash_init (table);

  /* Insert items */
  for (int i = 0; i < 100; i++)
    {
      item = malloc (sizeof (*item));
      item->key = i;
      item->value = i;
      hash_add (table, i, &item->node);
    }

  /* Delete even values during iteration */
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    if (item->value % 2 == 0)
      {
        hlist_del (&item->node);
        free (item);
      }
  }

  /* Count remaining items */
  int count = 0;
  hash_for_each_entry (table, bucket, item, node)
  {
    count++;
  }
  ASSERT (count == 50);

  /* Verify only odd values remain */
  for (int i = 0; i < 100; i++)
    {
      struct test_item *found = NULL;
      hash_for_each_possible_entry (table, i, item, node)
      {
        if (item->key == i)
          {
            found = item;
            break;
          }
      }
      if (i % 2 == 0)
        ASSERT (found == NULL);
      else
        ASSERT (found != NULL);
    }

  /* Clean up */
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    hlist_del (&item->node);
    free (item);
  }
}

TEST (possible_iteration)
{
  HASHTABLE (table, 8);
  struct test_item *item;
  int found[10] = {0};

  hash_init (table);

  /* Insert multiple items with same key hash (keys 0, 256, 512...) */
  for (int i = 0; i < 5; i++)
    {
      item = malloc (sizeof (*item));
      item->key = i * 256; /* These will hash to same bucket */
      item->value = i;
      hash_add (table, item->key, &item->node);
    }

  /* Insert other items */
  for (int i = 0; i < 5; i++)
    {
      item = malloc (sizeof (*item));
      item->key = i + 1;
      item->value = i + 100;
      hash_add (table, item->key, &item->node);
    }

  /* Iterate only bucket for key 0 */
  struct hlist_node *pos;
  hash_for_each_possible (table, 0, pos)
  {
      struct test_item *it = container_of (pos, struct test_item, node);
      found[it->value] = 1;
  }

  /* Should find items with values 0-4 (those with key 0, 256, 512...) */
  for (int i = 0; i < 5; i++)
    {
      ASSERT (found[i] == 1);
    }

  /* Clean up */
  unsigned bucket;
  struct hlist_node *tmp;
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    hlist_del (&item->node);
    free (item);
  }
}

TEST (stress_many_items)
{
  HASHTABLE (table, 12); /* 4096 buckets */
  struct test_item **items;
  int n = 100000;

  items = malloc (n * sizeof (*items));
  ASSERT (items != NULL);

  hash_init (table);

  /* Insert many items */
  for (int i = 0; i < n; i++)
    {
      items[i] = malloc (sizeof (*items[i]));
      items[i]->key = i;
      items[i]->value = i;
      hash_add (table, i, &items[i]->node);
    }

  /* Count items */
  int count = 0;
  struct test_item *item;
  unsigned bucket;
  hash_for_each_entry (table, bucket, item, node)
  {
    count++;
  }
  ASSERT (count == n);

  /* Find all items */
  for (int i = 0; i < n; i++)
    {
      struct test_item *found = NULL;
      hash_for_each_possible_entry (table, i, item, node)
      {
        if (item->key == i)
          {
            found = item;
            break;
          }
      }
      ASSERT (found != NULL);
      ASSERT (found->value == i);
    }

  /* Delete all items */
  struct hlist_node *tmp;
  hash_for_each_entry_safe (table, bucket, item, tmp, node)
  {
    hlist_del (&item->node);
    free (item);
  }

  free (items);
}

TEST (hash_size_bits)
{
  HASHTABLE (table8, 8);
  HASHTABLE (table4, 4);
  HASHTABLE (table12, 12);

  ASSERT (HASH_SIZE (table8) == 256);
  ASSERT (HASH_SIZE (table4) == 16);
  ASSERT (HASH_SIZE (table12) == 4096);

  ASSERT (HASH_BITS (table8) == 8);
  ASSERT (HASH_BITS (table4) == 4);
  ASSERT (HASH_BITS (table12) == 12);
}

TEST (empty_table)
{
  HASHTABLE (table, 8);
  struct test_item *item;
  unsigned bucket;

  hash_init (table);

  /* Count should be 0 */
  int count = 0;
  hash_for_each_entry (table, bucket, item, node)
  {
    count++;
  }
  ASSERT (count == 0);

  /* Try to find nonexistent item */
  struct test_item *found = NULL;
  hash_for_each_possible_entry (table, 0, item, node)
  {
    if (item->key == 0)
      {
        found = item;
        break;
      }
  }
  ASSERT (found == NULL);

  /* Iterating empty table should work */
  count = 0;
  hash_for_each_entry (table, bucket, item, node)
  {
    count++;
  }
  ASSERT (count == 0);
}

TEST (update_existing)
{
  HASHTABLE (table, 8);

  hash_init (table);

  /* Insert first item with key 42 */
  struct test_item *item1 = malloc (sizeof (*item1));
  item1->key = 42;
  item1->value = 100;
  hash_add (table, 42, &item1->node);

  /* Insert second item with same key */
  struct test_item *item2 = malloc (sizeof (*item2));
  item2->key = 42;
  item2->value = 200;
  hash_add (table, 42, &item2->node);

  /* Both should exist in the bucket */
  int count = 0;
  struct test_item *item;
  hash_for_each_possible_entry (table, 42, item, node)
  {
    ASSERT (item->key == 42);
    count++;
  }
  ASSERT (count == 2);

  /* Clean up */
  hlist_del (&item1->node);
  hlist_del (&item2->node);
  free (item1);
  free (item2);
}

int
main (void)
{
  printf ("=== Hash Table Comprehensive Test Suite ===\n\n");

  RUN_TEST (basic_operations);
  RUN_TEST (collisions);
  RUN_TEST (delete_from_bucket);
  RUN_TEST (iterate_all_buckets);
  RUN_TEST (safe_iteration_with_delete);
  RUN_TEST (possible_iteration);
  RUN_TEST (stress_many_items);
  RUN_TEST (hash_size_bits);
  RUN_TEST (empty_table);
  RUN_TEST (update_existing);

  printf ("\n=== Results ===\n");
  printf ("Passed: %d/%d\n", pass_count, test_count);

  return (pass_count == test_count) ? 0 : 1;
}
