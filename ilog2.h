#ifndef ILOG2_H
#define ILOG2_H

#include "fls.h"
#include <stdint.h>

static inline int __attribute__ ((const)) ilog2_u32 (uint32_t val)
{
  return fls (val) - 1;
}

#endif /* ILOG2_H */
