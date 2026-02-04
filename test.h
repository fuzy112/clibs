/* test.h - Common test framework for clibs
 *
 * Copyright © 2026  Zhengyi Fu <i@fuzy.me>
 *
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

#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>

static int test_count = 0;
static int pass_count = 0;

#define TEST(name) static void test_##name(void)

#define RUN_TEST(name)                                                        \
  do                                                                          \
    {                                                                         \
      fprintf(stderr, "Running %s... ", #name);                               \
      fflush(stderr);                                                         \
      test_count++;                                                           \
      test_##name();                                                          \
      fprintf(stderr, "PASS\n");                                              \
      pass_count++;                                                           \
    }                                                                         \
  while (0)

#define ASSERT(cond)                                                          \
  do                                                                          \
    {                                                                         \
      if (!(cond))                                                            \
        {                                                                     \
          fprintf(stderr, "\n  ASSERTION FAILED: %s at line %d\n", #cond,     \
                  __LINE__);                                                  \
          fflush(stdout);                                                     \
          fflush(stderr);                                                     \
          abort();                                                            \
        }                                                                     \
    }                                                                         \
  while (0)

#define FAIL(msg)                                                             \
  do                                                                          \
    {                                                                         \
      fprintf(stderr, "\n  FAILED: %s at line %d\n", msg, __LINE__);          \
      fflush(stdout);                                                         \
      fflush(stderr);                                                         \
      abort();                                                                \
    }                                                                         \
  while (0)

#endif /* TEST_H */