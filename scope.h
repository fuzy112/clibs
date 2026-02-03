/* Copyright © 2026  Zhengyi Fu <i@fuzy.me> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef SCOPE_H
#define SCOPE_H

#include "defs.h"
#include <stdlib.h>

/*
 * Scope-based resource management utilities
 *
 * Inspired by Linux kernel's scope-based resource management:
 * https://lwn.net/Articles/934679/
 */

/*
 * __cleanup() is already defined in defs.h
 * This is just for documentation purposes
 */

/*
 * freep() - Free a pointer and set it to NULL
 *
 * @p:      Pointer to the pointer to free (passed as void*)
 *
 * This function accepts a void* argument and casts it internally to void**.
 * This allows the same function to be used with __cleanup for any pointer type.
 * Use: char *ptr __cleanup(freep) = malloc(100);
 *      int *arr __cleanup(freep) = malloc(sizeof(int) * 10);
 */
static inline void freep(void *p)
{
	void **pp = (void **)p;
	if (pp && *pp) {
		free(*pp);
		*pp = NULL;
	}
}

/*
 * DEFINE_FREE() - Define a cleanup function for automatic memory management
 *
 * @name:   Identifier for this cleanup function
 * @type:   Type of the pointer to be cleaned up
 * @free:   Expression to free the resource (use _T for the pointer)
 *
 * Example:
 *   DEFINE_FREE(myfree, void *, if (_T) my_custom_free(_T))
 */
#define DEFINE_FREE(name, type, free)				\
	static inline void __free_##name(void *p)		\
	{							\
		type _T = *(type *)p;				\
		free;						\
	}

/*
 * __free() - Use a previously defined cleanup function
 *
 * @name:   Identifier of the cleanup function defined with DEFINE_FREE
 */
#define __free(name)	__cleanup(__free_##name)

/*
 * no_free_ptr() - Transfer ownership of a pointer (prevent automatic cleanup)
 *
 * @p:      Pointer to transfer ownership from
 *
 * Returns: The pointer value
 * Sets the original pointer to NULL to prevent double-free
 */
#define no_free_ptr(p)						\
	({ __auto_type __ptr = (p); (p) = NULL; __ptr; })

/*
 * return_ptr() - Return a pointer while preventing automatic cleanup
 *
 * @p:      Pointer to return
 *
 * Use this when returning a pointer from a function where it would
 * otherwise be automatically freed.
 */
#define return_ptr(p)	return no_free_ptr(p)

/*
 * DEFINE_CLASS() - Define a resource management class
 *
 * @name:       Identifier for this class
 * @type:       Type of the resource
 * @exit:       Expression to release the resource (use _T for the resource)
 * @init:       Expression to initialize the resource
 * @init_args:  Arguments for the initialization expression
 *
 * Example:
 *   DEFINE_CLASS(fd, int, close(_T), open(path, flags), const char *path, int flags)
 */
#define DEFINE_CLASS(name, type, exit, init, init_args...)	\
	typedef type class_##name##_t;				\
	static inline void class_##name##_destructor(type *p)	\
		{ type _T = *p; exit; }				\
	static inline type class_##name##_constructor(init_args)	\
		{ type t = init; return t; }

/*
 * CLASS() - Declare a variable of a resource management class
 *
 * @name:   Class name defined with DEFINE_CLASS
 * @var:    Variable name
 *
 * Note: This macro expands to an incomplete statement. You must provide
 *       constructor arguments after it.
 *
 * Example:
 *   CLASS(fd, f)(path, O_RDONLY);
 */
#define CLASS(name, var)						\
	class_##name##_t var __cleanup(class_##name##_destructor) =	\
		class_##name##_constructor

/*
 * __UNIQUE_ID() - Generate a unique identifier
 *
 * @prefix: Prefix for the identifier
 *
 * This uses __COUNTER__ to generate unique names.
 */
#ifndef __UNIQUE_ID
#define __UNIQUE_ID(prefix) __CONCAT(prefix, __COUNTER__)
#endif

#ifndef __CONCAT
#define __CONCAT(a, b) a ## b
#endif

/*
 * DEFINE_GUARD() - Define a lock guard class
 *
 * @name:   Identifier for this guard class
 * @type:   Type of the lock
 * @lock:   Expression to acquire the lock (use _T for the lock)
 * @unlock: Expression to release the lock (use _T for the lock)
 *
 * Example:
 *   DEFINE_GUARD(mutex, pthread_mutex_t *, pthread_mutex_lock(_T), pthread_mutex_unlock(_T))
 */
#define DEFINE_GUARD(name, type, lock, unlock)			\
	DEFINE_CLASS(name, type, unlock, ({ lock; _T; }), type _T)

/*
 * guard() - Create a lock guard instance
 *
 * @name:   Guard class name defined with DEFINE_GUARD
 *
 * Returns: An initialized guard instance
 * The lock will be automatically released when the guard goes out of scope.
 *
 * Example:
 *   guard(mutex)(&mutex);
 */
#define guard(name)							\
	CLASS(name, __UNIQUE_ID(guard))

#endif /* !SCOPE_H */