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

#include "b64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

TEST (encode_empty)
{
  char dest[10] = {0};
  size_t len = b64_encode (dest, "", 0);
  fprintf(stderr, "DEBUG: Checking len assertion...\n");
  ASSERT (len == 0);
  fprintf(stderr, "DEBUG: Checking dest[0] assertion...\n");
  ASSERT (dest[0] == '\0');
  fprintf(stderr, "DEBUG: Both assertions passed\n");
}

TEST (encode_single_char)
{
  char dest[10];
  size_t len = b64_encode (dest, "f", 1);
  ASSERT (len == 4);
  ASSERT (strcmp (dest, "Zg==") == 0);
}

TEST (encode_two_chars)
{
  char dest[10];
  size_t len = b64_encode (dest, "fo", 2);
  ASSERT (len == 4);
  ASSERT (strcmp (dest, "Zm8=") == 0);
}

TEST (encode_three_chars)
{
  char dest[10];
  size_t len = b64_encode (dest, "foo", 3);
  ASSERT (len == 4);
  ASSERT (strcmp (dest, "Zm9v") == 0);
}

TEST (encode_hello_world)
{
  char dest[32];
  size_t len = b64_encode (dest, "Hello, World!", 13);
  ASSERT (len == 20);
  ASSERT (strcmp (dest, "SGVsbG8sIFdvcmxkIQ==") == 0);
}

TEST (encode_all_bytes)
{
  char dest[345];
  unsigned char data[256];
  int i;

  for (i = 0; i < 256; i++)
    data[i] = i;

  size_t len = b64_encode (dest, data, 256);
  ASSERT (len == 344);
  ASSERT (dest[344] == '\0');
}

TEST (decode_empty)
{
  char dest[10];
  size_t len = b64_decode (dest, "", 0);
  ASSERT (len == 0);
}

TEST (decode_single_char)
{
  char dest[10];
  size_t len = b64_decode (dest, "Zg==", 4);
  ASSERT (len == 1);
  ASSERT (dest[0] == 'f');
}

TEST (decode_two_chars)
{
  char dest[10];
  size_t len = b64_decode (dest, "Zm8=", 4);
  ASSERT (len == 2);
  ASSERT (memcmp (dest, "fo", 2) == 0);
}

TEST (decode_three_chars)
{
  char dest[10];
  size_t len = b64_decode (dest, "Zm9v", 4);
  ASSERT (len == 3);
  ASSERT (memcmp (dest, "foo", 3) == 0);
}

TEST (decode_hello_world)
{
  char dest[20];
  size_t len = b64_decode (dest, "SGVsbG8sIFdvcmxkIQ==", 20);
  ASSERT (len == 13);
  ASSERT (memcmp (dest, "Hello, World!", 13) == 0);
}

TEST (roundtrip_simple)
{
  const char *original = "The quick brown fox jumps over the lazy dog";
  char encoded[100];
  char decoded[100];
  size_t enc_len, dec_len;

  enc_len = b64_encode (encoded, original, strlen (original));
  dec_len = b64_decode (decoded, encoded, enc_len);

  ASSERT (dec_len == strlen (original));
  ASSERT (memcmp (decoded, original, strlen (original)) == 0);
}

TEST (roundtrip_binary)
{
  unsigned char original[256];
  char encoded[400];
  unsigned char decoded[300];
  size_t enc_len, dec_len;
  int i;

  for (i = 0; i < 256; i++)
    original[i] = i;

  enc_len = b64_encode (encoded, original, 256);
  dec_len = b64_decode (decoded, encoded, enc_len);

  ASSERT (dec_len == 256);
  ASSERT (memcmp (decoded, original, 256) == 0);
}

TEST (roundtrip_various_lengths)
{
  char original[100];
  char encoded[200];
  char decoded[100];
  size_t enc_len, dec_len;
  int len, i;

  for (len = 1; len <= 100; len++)
    {
      for (i = 0; i < len; i++)
        original[i] = (char) (i % 256);

      enc_len = b64_encode (encoded, original, len);
      dec_len = b64_decode (decoded, encoded, enc_len);

      ASSERT (dec_len == (size_t) len);
      ASSERT (memcmp (decoded, original, len) == 0);
    }
}

TEST (padding_one_equals)
{
  char dest[10];
  size_t len = b64_decode (dest, "Zg=", 3);
  /* Note: This is technically malformed, but the decode handles it */
  /* We just verify it doesn't crash - len may be 0 or garbage */
  (void) len; /* Suppress unused variable warning */
}

TEST (long_string)
{
  char original[1000];
  char encoded[1400];
  char decoded[1000];
  size_t enc_len, dec_len;
  int i;

  for (i = 0; i < 1000; i++)
    original[i] = (char) ('A' + (i % 26));

  enc_len = b64_encode (encoded, original, 1000);
  dec_len = b64_decode (decoded, encoded, enc_len);

  ASSERT (dec_len == 1000);
  ASSERT (memcmp (decoded, original, 1000) == 0);
}

int
main (void)
{
  printf ("=== Base64 Encoding/Decoding Test Suite ===\n\n");

  RUN_TEST (encode_empty);
  RUN_TEST (encode_single_char);
  RUN_TEST (encode_two_chars);
  RUN_TEST (encode_three_chars);
  RUN_TEST (encode_hello_world);
  RUN_TEST (encode_all_bytes);
  RUN_TEST (decode_empty);
  RUN_TEST (decode_single_char);
  RUN_TEST (decode_two_chars);
  RUN_TEST (decode_three_chars);
  RUN_TEST (decode_hello_world);
  RUN_TEST (roundtrip_simple);
  RUN_TEST (roundtrip_binary);
  RUN_TEST (roundtrip_various_lengths);
  RUN_TEST (padding_one_equals);
  RUN_TEST (long_string);

  printf ("\n=== Results ===\n");
  printf ("Passed: %d/%d\n", pass_count, test_count);

  return (pass_count == test_count) ? 0 : 1;
}
