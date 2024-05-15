/* urlencode.h
 *
 * Copyright 2022, 2024 Zhengyi Fu <i@fuzy.me>
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

#ifndef ENCODE_URL_H
#define ENCODE_URL_H

#define URLENCODE_NO_RESV 0x1

char *encode_url (const char *url, int flags);
char *encode_url_component (const char *url, int flags);

#endif // ENCODE_URL_H
