/* encode_url.c
 *
 * Copyright 2022-2024 Zhengyi Fu <i@fuzy.me>
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

#include "encode_url.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool
is_unreserved (char ch)
{
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
         || (ch >= '0' && ch <= '9') || (ch == '-') || (ch == '_')
         || (ch == '.') || (ch == '~');
}

static bool
is_reserved (char ch)
{
  return ch == '!' || ch == '#' || ch == '$' || ch == '&' || ch == '\''
         || ch == '(' || ch == ')' || ch == '*' || ch == '+' || ch == ','
         || ch == '/' || ch == ':' || ch == ';' || ch == '=' || ch == '?'
         || ch == '@' || ch == '[' || ch == ']';
}

static const char xdigits[16] = {
  [0] = '0',   [1] = '1',   [2] = '2',   [3] = '3',   [4] = '4',   [5] = '5',
  [6] = '6',   [7] = '7',   [8] = '8',   [9] = '9',   [0xA] = 'A', [0xB] = 'B',
  [0xC] = 'C', [0xD] = 'D', [0xE] = 'E', [0xF] = 'F',
};

char *
encode_url (const char *url, int flags)
{
  return encode_url_component (url, flags);
}

char *
encode_url_component (const char *url, int flags)
{
  char *encoded;
  const char *p;
  size_t i;
  size_t len = 0;

  for (p = url; *p != '\0'; ++p)
    {
      unsigned char ch = *p;

      if (is_unreserved (ch))
        len += 1;
      else
        len += 3;
    }

  encoded = malloc (len + 1);
  if (encoded == NULL)
    return NULL;

  for (p = url, i = 0; *p != '\0'; ++p)
    {
      unsigned char ch = *p;

      if (is_unreserved (ch))
        {
          encoded[i++] = ch;
        }
      else if ((flags & URLENCODE_NO_RESV) && is_reserved (ch))
        {
          encoded[i++] = ch;
        }
      else
        {
          encoded[i++] = '%';
          encoded[i++] = xdigits[(ch / 16)];
          encoded[i++] = xdigits[(ch % 16)];
        }
    }
  encoded[i] = '\0';

  return encoded;
}
