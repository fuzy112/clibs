/* Copyright © 2026  Zhengyi Fu <i@fuzy.me> */
/*
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CONTAINER_OF_H
#define CONTAINER_OF_H

#include <stddef.h>

#ifndef offsetof
#define offsetof(type, member) ((ptrdiff_t) & ((type *)0)->member)
#endif

/**
 * __container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 * WARNING: any const qualifier of @ptr is lost.
 * Do not use __container_of() in new code - use container_of() instead.
 */
#define __container_of(ptr, type, member) ({\
	void *__mptr = (void *)(ptr);\
	((type *)(__mptr - offsetof(type, member))); })

/* Check if ptr is const - works with NULL pointers too */
#define __is_const_ptr(ptr) \
	__builtin_types_compatible_p(typeof(ptr), const typeof(0 ? (ptr) : (ptr)) *)

/**
 * container_of - cast a member of a structure out to the containing
 *		structure and preserve the const-ness of the pointer
 * @ptr:		the pointer to the member
 * @type:		the type of the container struct this is embedded in.
 * @member:		the name of the member within the struct.
 *
 * This macro preserves const qualifiers. If @ptr is a const pointer,
 * the result will be a const pointer to the containing structure.
 */
#define container_of(ptr, type, member) \
	(__builtin_choose_expr(__is_const_ptr(ptr),\
		(const type *)__container_of(ptr, type, member),\
		(type *)__container_of(ptr, type, member)))

#endif /* !CONTAINER_OF_H */
