#ifndef CONTAINER_OF_H
#define CONTAINER_OF_H

#include <stddef.h>

#ifndef offsetof
#define offsetof(type, member) ((ptrdiff_t) & ((type *)0)->member)
#endif

#ifndef container_of
#ifdef __GNUC__
#define container_of(ptr, type, member)                                       \
  ({                                                                          \
    typeof (((type *)0)->member) *_mptr = (ptr);                              \
    (type *)((char *)_mptr - offsetof (type, member));                        \
  })
#else
#define container_of(ptr, type, member)                                       \
  ((type *)((char *)(ptr)-offsetof (type, member)))
#endif
#endif

#endif /* !CONTAINER_OF_H */
