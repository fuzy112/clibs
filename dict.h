/* dict.h
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

#ifndef DICT_H
#define DICT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dict *dict_t;

extern int dict_create(dict_t *, void dict_free_value(void *), void *dict_clone_value(const void *));

extern void dict_destroy(dict_t);

extern int dict_set(dict_t, const char *, const void *);

extern void *dict_get(dict_t, const char *);

#ifdef __cplusplus
}
#endif

#endif // !DICT_H
