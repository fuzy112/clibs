/* b64.c
 *
 * Copyright 2023, 2024 Zhengyi Fu <i@fuzy.me>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "b64.h"
#include <stdint.h>
#include <string.h>

static int
decode_b64 (const char src[static 4])
{
  int val = 0;
  int i;

  for (i = 0; i < 4; i++)
    {
      if (src[i] >= 'A' && src[i] <= 'Z')
        val |= (src[i] - 'A') << (6 * (3 - i));
      else if (src[i] >= 'a' && src[i] <= 'z')
        val |= (src[i] - 'a' + 26) << (6 * (3 - i));
      else if (src[i] >= '0' && src[i] <= '9')
        val |= (src[i] - '0' + 52) << (6 * (3 - i));
      else if (src[i] == '+')
        val |= 62 << (6 * (3 - i));
      else if (src[i] == '/')
        val |= 63 << (6 * (3 - i));
      else if (src[i] == '=')
        val |= 0 << (6 * (3 - i));
      else
        return -1;
    }
  return val;
}

static void
encode_b64 (char dest[static 4], const uint8_t src[static 3])
{
  const uint8_t input[] = {
    (src[0] >> 2) & 63,
    ((src[0] << 4) | (src[1] >> 4)) & 63,
    ((src[1] << 2) | (src[2] >> 6)) & 63,
    src[2] & 63,
  };
  int i;

  for (i = 0; i < 4; i++)
    {
      if (input[i] < 26)
        dest[i] = 'A' + input[i];
      else if (input[i] < 52)
        dest[i] = 'a' + (input[i] - 26);
      else if (input[i] < 62)
        dest[i] = '0' + (input[i] - 52);
      else if (input[i] == 62)
        dest[i] = '+';
      else
        dest[i] = '/';
    }
}

size_t
b64_encode (char *dest, const void *__restrict src, size_t len)
{
  const uint8_t *s = src;
  uint8_t tmp[3] = { 0 };

  if (len == 0)
    {
      dest[0] = '\0';
      return 0;
    }

  size_t full = len / 3;
  size_t rem = len % 3;
  size_t i;

  for (i = 0; i < full; ++i)
    encode_b64 (&dest[i * 4], &s[i * 3]);

  if (rem > 0)
    {
      memcpy (tmp, &s[full * 3], rem);
      encode_b64 (&dest[full * 4], tmp);
      if (rem == 1)
        {
          dest[full * 4 + 2] = '=';
          dest[full * 4 + 3] = '=';
        }
      else if (rem == 2)
        {
          dest[full * 4 + 3] = '=';
        }
    }

  dest[full * 4 + (rem > 0 ? 4 : 0)] = '\0';
  return full * 4 + (rem > 0 ? 4 : 0);
}

size_t
b64_decode (void *dest, const char *__restrict src, size_t len)
{
  size_t i;
  unsigned int val;
  uint8_t *d = dest;

  /* Handle empty input */
  if (len == 0)
    return 0;

  /* Calculate number of complete 4-byte chunks */
  size_t chunks = len / 4;
  if (chunks == 0)
    return 0;

  /* Process all but the last chunk */
  for (i = 0; i < chunks - 1; ++i)
    {
      val = decode_b64 (&src[i * 4]);
      d[i * 3] = (val >> 16) & 0xff;
      d[i * 3 + 1] = (val >> 8) & 0xff;
      d[i * 3 + 2] = val & 0xff;
    }

  /* Handle the last chunk (may have padding) */
  size_t last_chunk_idx = chunks - 1;
  val = decode_b64 (&src[last_chunk_idx * 4]);

  /* Check for padding */
  if (src[len - 1] == '=')
    {
      if (src[len - 2] == '=')
	{
	  /* Two padding chars: 1 output byte */
	  d[last_chunk_idx * 3] = (val >> 16) & 0xff;
	  return last_chunk_idx * 3 + 1;
	}
      else
	{
	  /* One padding char: 2 output bytes */
	  d[last_chunk_idx * 3] = (val >> 16) & 0xff;
	  d[last_chunk_idx * 3 + 1] = (val >> 8) & 0xff;
	  return last_chunk_idx * 3 + 2;
	}
    }
  else
    {
      /* No padding: 3 output bytes */
      d[last_chunk_idx * 3] = (val >> 16) & 0xff;
      d[last_chunk_idx * 3 + 1] = (val >> 8) & 0xff;
      d[last_chunk_idx * 3 + 2] = val & 0xff;
      return last_chunk_idx * 3 + 3;
    }
}
