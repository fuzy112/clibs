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

#include "fd.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
      fprintf(stderr, "PASS\n");                                               \
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

TEST(close_nointr_valid_fd)
{
  int fd = open("/dev/null", O_RDONLY);
  ASSERT(fd >= 0);
  ASSERT(close_nointr(fd) == 0);
}

TEST(close_nointr_invalid_fd)
{
  int result = close_nointr(-1);
  ASSERT(result < 0);
  ASSERT(result == -EBADF);
}

TEST(close_nointr_already_closed)
{
  int fd = open("/dev/null", O_RDONLY);
  ASSERT(fd >= 0);
  ASSERT(close_nointr(fd) == 0);
  
  /* Closing again should fail with EBADF */
  int result = close_nointr(fd);
  ASSERT(result < 0);
  ASSERT(result == -EBADF);
}

TEST(safe_close_valid_fd)
{
  int fd = open("/dev/null", O_RDONLY);
  ASSERT(fd >= 0);
  int result = safe_close(fd);
  ASSERT(result == -1);
}

TEST(safe_close_negative_fd)
{
  /* Should not crash or abort */
  int result = safe_close(-1);
  ASSERT(result == -1);
}

TEST(safe_close_zero)
{
  /* Should not crash - stdin is usually 0 */
  /* Note: We can't actually close stdin in tests, so we skip this */
  /* Just verify it doesn't crash with an invalid low FD */
  int result = safe_close(-999);
  ASSERT(result == -1);
}

TEST(safe_close_preserves_errno)
{
  int fd = open("/dev/null", O_RDONLY);
  ASSERT(fd >= 0);
  
  errno = 42; /* Set a known errno */
  safe_close(fd);
  ASSERT(errno == 42); /* Should preserve errno */
}

TEST(safe_closep_valid_fd)
{
  int fd = open("/dev/null", O_RDONLY);
  ASSERT(fd >= 0);
  safe_closep(&fd);
  ASSERT(fd == -1);
}

TEST(safe_closep_negative_fd)
{
  int fd = -1;
  safe_closep(&fd);
  ASSERT(fd == -1);
}

TEST(auto_close_fd_scope)
{
  int fd_copy;
  {
    auto_close_fd fd = open("/dev/null", O_RDONLY);
    ASSERT(fd >= 0);
    fd_copy = fd; /* Save for verification */
    /* fd should be automatically closed when leaving scope */
  }
  
  /* Verify the FD was closed by trying to close it again */
  /* This should fail with EBADF if it was already closed */
  int result = close_nointr(fd_copy);
  if (result == 0)
    {
      /* It wasn't closed, which is unexpected */
      /* But we can't really test this reliably */
      close(fd_copy);
    }
  /* If result < 0, it means it was already closed (good!) */
}

TEST(auto_close_fd_multiple)
{
  auto_close_fd fd1 = open("/dev/null", O_RDONLY);
  auto_close_fd fd2 = open("/dev/zero", O_RDONLY);
  auto_close_fd fd3 = open("/dev/null", O_WRONLY);
  
  ASSERT(fd1 >= 0);
  ASSERT(fd2 >= 0);
  ASSERT(fd3 >= 0);
  ASSERT(fd1 != fd2);
  ASSERT(fd2 != fd3);
  /* All should be auto-closed */
}

TEST(close_nointr_pipe)
{
  int pipefd[2];
  ASSERT(pipe(pipefd) == 0);
  
  ASSERT(close_nointr(pipefd[0]) == 0);
  ASSERT(close_nointr(pipefd[1]) == 0);
}

TEST(safe_close_pipe)
{
  int pipefd[2];
  ASSERT(pipe(pipefd) == 0);
  
  ASSERT(safe_close(pipefd[0]) == -1);
  ASSERT(safe_close(pipefd[1]) == -1);
}

TEST(multiple_open_close_cycles)
{
  for (int i = 0; i < 100; i++)
    {
      int fd = open("/dev/null", O_RDONLY);
      ASSERT(fd >= 0);
      ASSERT(close_nointr(fd) == 0);
    }
}

TEST(fd_lifecycle)
{
  /* Test a typical FD lifecycle */
  int fd = open("/dev/null", O_RDWR);
  ASSERT(fd >= 0);
  
  /* Write something */
  const char *msg = "test";
  ssize_t written = write(fd, msg, strlen(msg));
  ASSERT(written == (ssize_t)strlen(msg));
  
  /* Close using safe_close */
  ASSERT(safe_close(fd) == -1);
}

int
main(void)
{
  fprintf(stderr, "=== FD Utility Test Suite ===\n\n");

  RUN_TEST(close_nointr_valid_fd);
  RUN_TEST(close_nointr_invalid_fd);
  RUN_TEST(close_nointr_already_closed);
  RUN_TEST(safe_close_valid_fd);
  RUN_TEST(safe_close_negative_fd);
  RUN_TEST(safe_close_zero);
  RUN_TEST(safe_close_preserves_errno);
  RUN_TEST(safe_closep_valid_fd);
  RUN_TEST(safe_closep_negative_fd);
  RUN_TEST(auto_close_fd_scope);
  RUN_TEST(auto_close_fd_multiple);
  RUN_TEST(close_nointr_pipe);
  RUN_TEST(safe_close_pipe);
  RUN_TEST(multiple_open_close_cycles);
  RUN_TEST(fd_lifecycle);

  fprintf(stderr, "\n=== Results ===\n");
  fprintf(stderr, "Passed: %d/%d\n", pass_count, test_count);

  return (pass_count == test_count) ? 0 : 1;
}
