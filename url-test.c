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

#include "encode_url.h"
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
  char *result = encode_url ("", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "") == 0);
  free (result);
}

TEST (encode_unreserved)
{
  /* Unreserved chars should not be encoded */
  char *result = encode_url ("abcABC123-_.~", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "abcABC123-_.~") == 0);
  free (result);
}

TEST (encode_space)
{
  char *result = encode_url ("hello world", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "hello%20world") == 0);
  free (result);
}

TEST (encode_special_chars)
{
  char *result = encode_url ("foo@bar.com", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "foo%40bar.com") == 0);
  free (result);
}

TEST (encode_url_example)
{
  char *result = encode_url ("https://example.com/path?key=value", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result,
                  "https%3A%2F%2Fexample.com%2Fpath%3Fkey%3Dvalue")
          == 0);
  free (result);
}

TEST (encode_unicode)
{
  /* UTF-8 characters should be encoded byte-by-byte */
  char *result = encode_url ("café", 0);
  ASSERT (result != NULL);
  /* é in UTF-8 is 0xC3 0xA9 */
  ASSERT (strcmp (result, "caf%C3%A9") == 0);
  free (result);
}

TEST (encode_all_ascii)
{
  /* Test encoding of all ASCII characters */
  char input[128];
  char *result;
  int i;

  for (i = 0; i < 127; i++)
    input[i] = (char) (i + 1);
  input[127] = '\0';

  result = encode_url (input, 0);
  ASSERT (result != NULL);
  /* Should be significantly longer due to encoding */
  ASSERT (strlen (result) > 127);
  free (result);
}

TEST (encode_with_no_reserved_flag)
{
  /* URLENCODE_NO_RESV should preserve reserved chars */
  char *result = encode_url ("hello/world?test=1", URLENCODE_NO_RESV);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "hello/world?test=1") == 0);
  free (result);
}

TEST (encode_component_vs_url)
{
  /* Both functions should behave the same currently */
  const char *input = "test/path?q=1";
  char *url_result = encode_url (input, 0);
  char *comp_result = encode_url_component (input, 0);

  ASSERT (url_result != NULL);
  ASSERT (comp_result != NULL);
  ASSERT (strcmp (url_result, comp_result) == 0);

  free (url_result);
  free (comp_result);
}

TEST (encode_null_bytes)
{
  /* String with embedded null should stop at null */
  char input[] = "test\0more";
  char *result = encode_url (input, 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "test") == 0);
  free (result);
}

TEST (encode_mixed)
{
  char *result = encode_url ("Hello World! How are you?", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "Hello%20World%21%20How%20are%20you%3F") == 0);
  free (result);
}

TEST (encode_hash)
{
  char *result = encode_url ("section#header", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "section%23header") == 0);
  free (result);
}

TEST (encode_ampersand)
{
  char *result = encode_url ("a&b", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "a%26b") == 0);
  free (result);
}

TEST (encode_plus)
{
  char *result = encode_url ("1+1=2", 0);
  ASSERT (result != NULL);
  /* + is reserved but encoded by default */
  ASSERT (strcmp (result, "1%2B1%3D2") == 0);
  free (result);
}

TEST (encode_percent)
{
  char *result = encode_url ("100% complete", 0);
  ASSERT (result != NULL);
  ASSERT (strcmp (result, "100%25%20complete") == 0);
  free (result);
}

int
main (void)
{
  printf ("=== URL Encoding Test Suite ===\n\n");

  RUN_TEST (encode_empty);
  RUN_TEST (encode_unreserved);
  RUN_TEST (encode_space);
  RUN_TEST (encode_special_chars);
  RUN_TEST (encode_url_example);
  RUN_TEST (encode_unicode);
  RUN_TEST (encode_all_ascii);
  RUN_TEST (encode_with_no_reserved_flag);
  RUN_TEST (encode_component_vs_url);
  RUN_TEST (encode_null_bytes);
  RUN_TEST (encode_mixed);
  RUN_TEST (encode_hash);
  RUN_TEST (encode_ampersand);
  RUN_TEST (encode_plus);
  RUN_TEST (encode_percent);

  printf ("\n=== Results ===\n");
  printf ("Passed: %d/%d\n", pass_count, test_count);

  return (pass_count == test_count) ? 0 : 1;
}
