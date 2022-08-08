/* quark.h
 *
 * Copyright 2022 Zhengyi Fu <tsingyat@outlook.com>
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


#ifndef QUARK_H
#define QUARK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef long quark_t;

void quark_init();
void quark_exit();

quark_t quark_from_str(const char *str);

const char *quark_to_str(quark_t quark);

const char *quark_intern(const char *str);

#ifdef __cplusplus
}
#endif


#endif // !QUARK_H
