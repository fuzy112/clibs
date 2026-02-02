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


#ifndef FD_H
#define FD_H

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "defs.h"

static inline int close_nointr(int fd)
{
  if (close(fd) >= 0)
    return 0;

  if (errno == EINTR)
    return 0;

  return -errno;
}

static inline int safe_close(int fd)
{
  if (fd >= 0) {
    int error = errno;

    if (close_nointr(fd) == -EBADF)
      abort();

    errno = error;
  }

  return -1;
}

static inline void safe_closep(int *fd)
{
  *fd = safe_close(*fd);
}

#define auto_close_fd int __cleanup(safe_closep)


#endif /* !FD_H */
