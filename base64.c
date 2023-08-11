/* base64.c
 *
 * Copyright 2023 Zhengyi Fu <tsingyat@outlook.com>
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

#include "base64.h"
#include <stdint.h>
#include <string.h>

static int
decode_base64 (const char src[static 4])
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
encode_base64 (char dest[static 4], const uint8_t src[static 3])
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
base64_encode (char *dest, const void *restrict src, size_t len)
{
  const uint8_t *s = src;
  size_t i;
  uint8_t tmp[3] = { 0 };

  for (i = 0; i < len / 3; ++i)
    encode_base64 (&dest[i * 4], &s[i * 3]);

  memcpy (tmp, &s[i * 3], len % 3);
  encode_base64 (&dest[i * 4], tmp);
  ++i;
  if (len % 3 == 1)
    {
      dest[i * 4 - 2] = '=';
      dest[i * 4 - 1] = '=';
    }
  else if (len % 3 == 2)
    {
      dest[i * 4 - 1] = '=';
    }
  dest[i * 4] = '\0';
  return i * 4;
}

size_t
base64_decode (void *dest, const char *restrict src, size_t len)
{
  size_t i;
  unsigned int val;
  uint8_t *d = dest;

  for (i = 0; i < len / 4; ++i)
    {
      val = decode_base64 (&src[i * 4]);
      d[i * 3] = (val >> 16) & 0xff;
      d[i * 3 + 1] = (val >> 8) & 0xff;
      d[i * 3 + 2] = val & 0xff;
    }
  if (src[len - 1] == '=')
    {
      if (src[len - 2] == '=')
        {
          val = decode_base64 (&src[len - 4]);
          d[i * 3] = (val >> 16) & 0xff;
          d[i * 3 + 1] = (val >> 8) & 0xff;
        }
      else
        {
          val = decode_base64 (&src[len - 4]);
          d[i * 3] = (val >> 16) & 0xff;
        }
    }
  return i * 3;
}
