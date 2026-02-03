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

#ifndef SCOPE_GUARDS_H
#define SCOPE_GUARDS_H

#include "scope.h"
#include <pthread.h>
#include <threads.h>

/*
 * Convenience guard definitions for common locking primitives
 *
 * These pre-defined guards can be used directly without needing to
 * define them in each source file.
 */

/*
 * pthread_mutex_t guards
 */

/* Guard for pthread_mutex_t (pointer version) */
DEFINE_GUARD(mutex, pthread_mutex_t *, pthread_mutex_lock(_T), pthread_mutex_unlock(_T))

/* Guard for pthread_mutex_t (value version) */
DEFINE_GUARD(mutex_val, pthread_mutex_t, pthread_mutex_lock(&_T), pthread_mutex_unlock(&_T))

/*
 * pthread_rwlock_t guards
 */

/* Guard for pthread_rwlock_t read lock (pointer version) */
DEFINE_GUARD(rwlock_rd, pthread_rwlock_t *, pthread_rwlock_rdlock(_T), pthread_rwlock_unlock(_T))

/* Guard for pthread_rwlock_t write lock (pointer version) */
DEFINE_GUARD(rwlock_wr, pthread_rwlock_t *, pthread_rwlock_wrlock(_T), pthread_rwlock_unlock(_T))

/* Guard for pthread_rwlock_t read lock (value version) */
DEFINE_GUARD(rwlock_rd_val, pthread_rwlock_t, pthread_rwlock_rdlock(&_T), pthread_rwlock_unlock(&_T))

/* Guard for pthread_rwlock_t write lock (value version) */
DEFINE_GUARD(rwlock_wr_val, pthread_rwlock_t, pthread_rwlock_wrlock(&_T), pthread_rwlock_unlock(&_T))

/*
 * pthread_spinlock_t guards
 */

/* Guard for pthread_spinlock_t (pointer version) */
DEFINE_GUARD(spinlock, pthread_spinlock_t *, pthread_spin_lock(_T), pthread_spin_unlock(_T))

/* Note: No value version for pthread_spinlock_t due to volatile qualifier */

/*
 * Common memory management guards
 */

/* Use freep directly with __cleanup for automatic memory management */
/* Example: char *ptr __cleanup(freep) = malloc(100); */

/*
 * File descriptor guards
 */

#include <fcntl.h>
#include <unistd.h>

/* Guard for file descriptors (open/close) */
DEFINE_CLASS(fd, int, if (_T >= 0) close(_T), open(path, flags), const char *path, int flags)

/* Guard for file descriptors with mode (open/close) */
DEFINE_CLASS(fd_mode, int, if (_T >= 0) close(_T), open(path, flags, mode), const char *path, int flags, mode_t mode)

/*
 * FILE* guards
 */

#include <stdio.h>

/* Guard for FILE* (fopen/fclose) */
DEFINE_CLASS(file, FILE *, if (_T) fclose(_T), fopen(path, mode), const char *path, const char *mode)

/*
 * C11 thread guards
 */

/* Guard for mtx_t (pointer version) */
DEFINE_GUARD(mtx, mtx_t *, mtx_lock(_T), mtx_unlock(_T))

/* Guard for mtx_t (value version) */
DEFINE_GUARD(mtx_val, mtx_t, mtx_lock(&_T), mtx_unlock(&_T))

/* Guard for mtx_t with timeout (pointer version) */
DEFINE_CLASS(mtx_timed, mtx_t *, mtx_unlock(_T), ({ \
		struct timespec ts; \
		timespec_get(&ts, TIME_UTC); \
		ts.tv_sec += timeout_sec; \
		if (mtx_timedlock(_T, &ts) != thrd_success) { \
			_T = NULL; /* Mark as failed to acquire */ \
		} \
		_T; \
	}), mtx_t *_T, int timeout_sec)

/* Guard for mtx_t with timeout (value version) */
DEFINE_CLASS(mtx_timed_val, mtx_t, mtx_unlock(&_T), ({ \
		struct timespec ts; \
		timespec_get(&ts, TIME_UTC); \
		ts.tv_sec += timeout_sec; \
		if (mtx_timedlock(&_T, &ts) != thrd_success) { \
			/* Can't null a value, but we can mark it somehow */ \
		} \
		_T; \
	}), mtx_t _T, int timeout_sec)

/* Guard for cnd_t (pointer version) - for use with mtx */
DEFINE_GUARD(cnd, cnd_t *, (void)0, cnd_destroy(_T))

/* Guard for cnd_t (value version) */
DEFINE_GUARD(cnd_val, cnd_t, (void)0, cnd_destroy(&_T))

/* Simple guard for thrd_t - joins thread when going out of scope */
static inline void auto_join_thread(thrd_t *t)
{
	if (t && !thrd_equal(*t, thrd_current())) {
		thrd_join(*t, NULL);
	}
}

/* Macro to create a thread join guard */
#define AUTO_JOIN_THREAD(thrd) \
	thrd_t __auto_join_##thrd __cleanup(auto_join_thread) = thrd

/*
 * Convenience macros for common use cases
 */

/* Lock a pthread mutex with automatic unlock */
#define AUTO_MUTEX(mutex_ptr)		guard(mutex)(mutex_ptr)

/* Lock a pthread mutex by value with automatic unlock */
#define AUTO_MUTEX_VAL(mutex_var)	guard(mutex_val)(mutex_var)

/* Lock a C11 mtx_t with automatic unlock */
#define AUTO_MTX(mtx_ptr)		guard(mtx)(mtx_ptr)

/* Lock a C11 mtx_t by value with automatic unlock */
#define AUTO_MTX_VAL(mtx_var)		guard(mtx_val)(mtx_var)

/* Lock a C11 mtx_t with timeout */
#define AUTO_MTX_TIMED(mtx_ptr, timeout)	CLASS(mtx_timed, __UNIQUE_ID(mtx))(mtx_ptr, timeout)

/* Lock a C11 mtx_t by value with timeout */
#define AUTO_MTX_TIMED_VAL(mtx_var, timeout)	CLASS(mtx_timed_val, __UNIQUE_ID(mtx))(mtx_var, timeout)

/* Acquire pthread read lock with automatic unlock */
#define AUTO_RDLOCK(rwlock_ptr)		guard(rwlock_rd)(rwlock_ptr)

/* Acquire pthread write lock with automatic unlock */
#define AUTO_WRLOCK(rwlock_ptr)		guard(rwlock_wr)(rwlock_ptr)

/* Lock a pthread spinlock with automatic unlock */
#define AUTO_SPINLOCK(spinlock_ptr)	guard(spinlock)(spinlock_ptr)

/* Note: No AUTO_SPINLOCK_VAL due to volatile qualifier on pthread_spinlock_t */

/* Open file descriptor with automatic close */
#define AUTO_FD(path, flags)		CLASS(fd, __UNIQUE_ID(fd))(path, flags)

/* Open file descriptor with mode and automatic close */
#define AUTO_FD_MODE(path, flags, mode)	CLASS(fd_mode, __UNIQUE_ID(fd))(path, flags, mode)

/* Open FILE* with automatic close */
#define AUTO_FILE(path, mode)		CLASS(file, __UNIQUE_ID(file))(path, mode)

#endif /* !SCOPE_GUARDS_H */