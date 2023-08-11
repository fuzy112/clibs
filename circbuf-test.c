#include "circbuf.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
  circbuf_autoptr buf = circ_alloc (4096);
  struct iovec vec[2];
  unsigned nr_vecs = 0;
  // unsigned offset = 0;
  ssize_t len;
  bool input_closed = false;

  if (argc != 1)
    {
      fprintf (stderr, "Invalid arguments\n");
      exit (EXIT_FAILURE);
    }

  if (fcntl (STDIN_FILENO, F_SETFL, fcntl (STDIN_FILENO, F_GETFL) | O_NONBLOCK)
      < 0)
    goto err;

  while (1)
    {

      if (!input_closed)
        {
          circ_prepare (buf, vec, &nr_vecs);
          len = readv (STDIN_FILENO, vec, nr_vecs);
          if (len == -1)
            {
              if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                goto err;
              len = 0;
            }
          else if (len == 0)
            {
              input_closed = true;
            }

          circ_commit (buf, len);
        }

      if (circ_empty (buf) && input_closed)
        break;

      circ_data (buf, vec, &nr_vecs);
      len = writev (STDOUT_FILENO, vec, nr_vecs);
      if (len == -1)
        {
          goto err;
        }

      circ_consume (buf, len);
    }

  return 0;

err:
  perror (argv[0]);
  return 1;
}
