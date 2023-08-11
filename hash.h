#ifndef HASH_H
#define HASH_H

#include <stdint.h>

/*
 * This hash multiplies the input by a large odd number and takes the
 * high bits.  Since multiplication propagates changes to the most
 * significant end only, it is essential that the high bits of the
 * product be used for the hash value.
 *
 * Chuck Lever verified the effectiveness of this technique:
 * http://www.citi.umich.edu/techreports/reports/citi-tr-00-1.pdf
 *
 * Although a random odd number will do, it turns out that the golden
 * ratio phi = (sqrt(5)-1)/2, or its negative, has particularly nice
 * properties.  (See Knuth vol 3, section 6.4, exercise 9.)
 *
 * These are the negative, (1 - phi) = phi**2 = (3 - sqrt(5))/2,
 * which is very slightly easier to multiply by and makes no
 * difference to the hash distribution.
 */
#define GOLDEN_RATIO_32 0x61C88647
#define GOLDEN_RATIO_64 0x61C8864680B583EBull

static inline uint32_t __attribute__ ((const))
hash_64 (uint64_t val, unsigned int bits)
{
  return (val * GOLDEN_RATIO_64) >> (64 - bits);
}

static inline uint32_t __attribute__ ((const))
hash_32 (uint32_t val, unsigned int bits)
{
  return (val * GOLDEN_RATIO_32) >> (32 - bits);
}

static inline uint32_t __attribute__ ((const))
hash_long (unsigned long val, unsigned int bits)
{
  if (sizeof (long) == sizeof (uint32_t))
    return hash_32 (val, bits);
  else
    return hash_64 (val, bits);
}

#endif /* HASH_H */
