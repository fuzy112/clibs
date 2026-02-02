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

#include "circbuf.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

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

TEST (basic_create_destroy)
{
  struct circbuf *buf = circ_alloc (64);
  ASSERT (buf != NULL);
  ASSERT (circ_empty (buf));
  ASSERT (circ_count (buf) == 0);
  ASSERT (circ_space (buf) == 63); /* size - 1 */
  circ_free (buf);
}

TEST (invalid_size)
{
  /* Size must be power of 2 */
  ASSERT (circ_alloc (0) == NULL);
  ASSERT (circ_alloc (3) == NULL);
  ASSERT (circ_alloc (5) == NULL);
  ASSERT (circ_alloc (63) == NULL);
  ASSERT (circ_alloc (65) == NULL);
}

TEST (basic_write_read)
{
  struct circbuf *buf = circ_alloc (64);
  unsigned off = 0;
  const char *msg = "Hello, World!";
  char read_buf[64];

  ASSERT (circ_write (buf, msg, strlen (msg), &off) == 0);
  circ_commit (buf, off);

  ASSERT (circ_count (buf) == strlen (msg));
  ASSERT (!circ_empty (buf));

  memset (read_buf, 0, sizeof (read_buf));
  off = 0;
  ASSERT (circ_read (buf, read_buf, strlen (msg), &off) == 0);
  circ_consume (buf, off);

  ASSERT (strcmp (read_buf, msg) == 0);
  ASSERT (circ_empty (buf));

  circ_free (buf);
}

TEST (write_read_multiple)
{
  struct circbuf *buf = circ_alloc (256);
  unsigned off;
  int i;

  /* Write several messages */
  for (i = 0; i < 10; i++)
    {
      char msg[32];
      snprintf (msg, sizeof (msg), "Message %d", i);
      off = 0;
      ASSERT (circ_write (buf, msg, strlen (msg), &off) == 0);
      circ_commit (buf, off);
    }

  /* Read them back */
  for (i = 0; i < 10; i++)
    {
      char msg[32];
      char read_buf[32];
      snprintf (msg, sizeof (msg), "Message %d", i);

      off = 0;
      ASSERT (circ_read (buf, read_buf, strlen (msg), &off) == 0);
      circ_consume (buf, off);
    }

  ASSERT (circ_empty (buf));
  circ_free (buf);
}

TEST (wraparound)
{
  struct circbuf *buf = circ_alloc (32);
  unsigned off = 0;
  char data[20] = {0};
  char read_buf[20];

  /* Fill buffer partially */
  ASSERT (circ_write (buf, data, 20, &off) == 0);
  circ_commit (buf, off);

  /* Consume half */
  circ_consume (buf, 10);

  /* Write more (should wrap) */
  off = 0;
  ASSERT (circ_write (buf, data, 10, &off) == 0);
  circ_commit (buf, off);

  ASSERT (circ_count (buf) == 20);

  /* Read should handle wraparound */
  off = 0;
  ASSERT (circ_read (buf, read_buf, 20, &off) == 0);
  circ_consume (buf, off);

  ASSERT (circ_empty (buf));
  circ_free (buf);
}

TEST (write_full)
{
  struct circbuf *buf = circ_alloc (64);
  unsigned off = 0;
  char data[63]; /* Maximum that fits */

  memset (data, 'X', sizeof (data));

  /* Should succeed - fills exactly to capacity */
  ASSERT (circ_write (buf, data, 63, &off) == 0);
  circ_commit (buf, off);

  ASSERT (circ_count (buf) == 63);
  ASSERT (circ_space (buf) == 0);

  /* Should fail - no space left */
  off = 0;
  ASSERT (circ_write (buf, data, 1, &off) != 0);

  circ_free (buf);
}

TEST (read_beyond_data)
{
  struct circbuf *buf = circ_alloc (64);
  unsigned off = 0;
  char data[10] = {0};
  char read_buf[20];

  ASSERT (circ_write (buf, data, 10, &off) == 0);
  circ_commit (buf, off);

  /* Try to read more than available */
  off = 0;
  ASSERT (circ_read (buf, read_buf, 20, &off) != 0);

  /* Read exactly available should work */
  ASSERT (circ_read (buf, read_buf, 10, &off) == 0);

  circ_free (buf);
}

TEST (peek_without_consume)
{
  struct circbuf *buf = circ_alloc (64);
  unsigned off = 0;
  char msg[] = "Hello";
  char read_buf[10];

  ASSERT (circ_write (buf, msg, strlen (msg), &off) == 0);
  circ_commit (buf, off);

  /* Read without consuming */
  off = 0;
  ASSERT (circ_read (buf, read_buf, strlen (msg), &off) == 0);
  ASSERT (circ_count (buf) == strlen (msg)); /* Data still there */

  /* Read again with new offset */
  off = 0;
  memset (read_buf, 0, sizeof (read_buf));
  ASSERT (circ_read (buf, read_buf, strlen (msg), &off) == 0);
  ASSERT (strcmp (read_buf, msg) == 0);

  circ_free (buf);
}

TEST (iov_prepare)
{
  struct circbuf *buf = circ_alloc (64);
  struct iovec vec[2];
  unsigned nr_vecs;
  unsigned off = 0;

  /* Empty buffer - should return 0 vectors */
  circ_prepare (buf, vec, &nr_vecs);
  ASSERT (nr_vecs == 0);

  /* Add some data */
  char data[20] = {0};
  ASSERT (circ_write (buf, data, 20, &off) == 0);
  circ_commit (buf, off);

  /* Should return 1 vector for contiguous data */
  circ_prepare (buf, vec, &nr_vecs);
  ASSERT (nr_vecs == 1);
  ASSERT (vec[0].iov_len == 42); /* 63 - 20 - 1 = space left */

  circ_free (buf);
}

TEST (iov_data)
{
  struct circbuf *buf = circ_alloc (64);
  struct iovec vec[2];
  unsigned nr_vecs;
  unsigned off = 0;
  char data[20] = "Hello, World!";

  ASSERT (circ_write (buf, data, strlen (data), &off) == 0);
  circ_commit (buf, off);

  /* Should return data vectors */
  circ_data (buf, vec, &nr_vecs);
  ASSERT (nr_vecs >= 1);
  ASSERT (vec[0].iov_len == strlen (data));
  ASSERT (memcmp (vec[0].iov_base, data, strlen (data)) == 0);

  circ_free (buf);
}

TEST (u8_operations)
{
  struct circbuf *buf = circ_alloc (64);
  unsigned off = 0;
  uint8_t val = 0xAB;
  uint8_t read_val;

  ASSERT (circ_write_u8 (buf, val, &off) == 0);
  circ_commit (buf, off);

  off = 0;
  ASSERT (circ_read_u8 (buf, &read_val, &off) == 0);
  circ_consume (buf, off);

  ASSERT (read_val == val);
  circ_free (buf);
}

TEST (u16_operations)
{
  struct circbuf *buf = circ_alloc (64);
  unsigned off = 0;
  uint16_t val = 0xABCD;
  uint16_t read_be, read_le;

  /* Big endian */
  off = 0;
  ASSERT (circ_write_be16 (buf, val, &off) == 0);
  circ_commit (buf, off);

  off = 0;
  ASSERT (circ_read_be16 (buf, &read_be, &off) == 0);
  circ_consume (buf, off);
  ASSERT (read_be == val);

  /* Little endian */
  off = 0;
  ASSERT (circ_write_le16 (buf, val, &off) == 0);
  circ_commit (buf, off);

  off = 0;
  ASSERT (circ_read_le16 (buf, &read_le, &off) == 0);
  circ_consume (buf, off);
  ASSERT (read_le == val);

  circ_free (buf);
}

TEST (u32_operations)
{
  struct circbuf *buf = circ_alloc (64);
  unsigned off = 0;
  uint32_t val = 0xDEADBEEF;
  uint32_t read_be, read_le;

  /* Big endian */
  off = 0;
  ASSERT (circ_write_be32 (buf, val, &off) == 0);
  circ_commit (buf, off);

  off = 0;
  ASSERT (circ_read_be32 (buf, &read_be, &off) == 0);
  circ_consume (buf, off);
  ASSERT (read_be == val);

  /* Little endian */
  off = 0;
  ASSERT (circ_write_le32 (buf, val, &off) == 0);
  circ_commit (buf, off);

  off = 0;
  ASSERT (circ_read_le32 (buf, &read_le, &off) == 0);
  circ_consume (buf, off);
  ASSERT (read_le == val);

  circ_free (buf);
}

TEST (count_and_space)
{
  struct circbuf *buf = circ_alloc (32);
  unsigned off = 0;
  char data[10] = {0};

  ASSERT (circ_count (buf) == 0);
  ASSERT (circ_space (buf) == 31);

  ASSERT (circ_write (buf, data, 10, &off) == 0);
  circ_commit (buf, off);

  ASSERT (circ_count (buf) == 10);
  ASSERT (circ_space (buf) == 21);

  circ_consume (buf, 5);

  ASSERT (circ_count (buf) == 5);
  ASSERT (circ_space (buf) == 26);

  circ_free (buf);
}

TEST (stress_multiple_wraparounds)
{
  struct circbuf *buf = circ_alloc (256);
  int i;
  unsigned off;

  for (i = 0; i < 1000; i++)
    {
      char write_data[50];
      char read_data[50];

      snprintf (write_data, sizeof (write_data), "Iteration %d", i);

      off = 0;
      ASSERT (circ_write (buf, write_data, strlen (write_data), &off) == 0);
      circ_commit (buf, off);

      /* Consume some to make room */
      if (circ_count (buf) > 200)
        {
          off = 0;
          ASSERT (circ_read (buf, read_data, 50, &off) == 0);
          circ_consume (buf, off);
        }
    }

  /* Drain remaining */
  while (!circ_empty (buf))
    {
      char read_data[50];
      off = 0;
      if (circ_read (buf, read_data, 1, &off) == 0)
        circ_consume (buf, off);
      else
        break;
    }

  circ_free (buf);
}

int
main (void)
{
  printf ("=== Circular Buffer Comprehensive Test Suite ===\n\n");

  RUN_TEST (basic_create_destroy);
  RUN_TEST (invalid_size);
  RUN_TEST (basic_write_read);
  RUN_TEST (write_read_multiple);
  RUN_TEST (wraparound);
  RUN_TEST (write_full);
  RUN_TEST (read_beyond_data);
  RUN_TEST (peek_without_consume);
  RUN_TEST (iov_prepare);
  RUN_TEST (iov_data);
  RUN_TEST (u8_operations);
  RUN_TEST (u16_operations);
  RUN_TEST (u32_operations);
  RUN_TEST (count_and_space);
  RUN_TEST (stress_multiple_wraparounds);

  printf ("\n=== Results ===\n");
  printf ("Passed: %d/%d\n", pass_count, test_count);

  return (pass_count == test_count) ? 0 : 1;
}
