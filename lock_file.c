#include "lock_file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int
write_pid (int fd)
{
  char buf[128];
  int s;

  s = ftruncate (fd, 0);
  if (s < 0)
    return -1;

  s = snprintf (buf, sizeof (buf), "%lu", (long unsigned)getpid ());
  return write (fd, buf, s);
}

int
lock_file (int *lock, const char *filename)
{
  int fd;
  int s;

  if (*lock != LOCK_FILE_INIT)
    return -EINVAL;

  fd = open (filename, O_CREAT | O_CLOEXEC | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd < 0)
    return -errno;

  s = lockf (fd, F_LOCK, 0);
  if (s < 0)
    goto err;

  s = write_pid (fd);
  if (s < 0)
    goto err;

  *lock = fd;
  return fd;

err:
  s = -errno;
  if (close (fd) != 0)
    abort ();
  return s;
}

int
try_lock_file (int *lock, const char *filename)
{
  int fd, s;

  if (*lock != LOCK_FILE_INIT)
    return -EINVAL;

  fd = open (filename, O_CREAT | O_CLOEXEC | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd < 0)
    return -errno;

  if (lockf (fd, F_TLOCK, 0) < 0)
    {
      if (errno == EACCES || errno == EAGAIN)
        return -EAGAIN;

      goto err;
    }

  s = write_pid (fd);
  if (s < 0)
    goto err;

  *lock = fd;
  return fd;

err:
  s = -errno;
  if (close (fd) != 0)
    abort ();
  return s;
}

void
unlock_file (int *lock)
{
  int s;

  if (*lock == LOCK_FILE_INIT)
    return;

  ftruncate (*lock, 0);
  lockf (*lock, F_ULOCK, 0);
  close (*lock);

  *lock = LOCK_FILE_INIT;
}
