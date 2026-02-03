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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "scope-guards.h"

/*
 * Test C11 thread primitives with scope-based guards
 */

static mtx_t c11_mutex;
static int c11_shared_counter = 0;

/* Thread function using C11 threads */
static int thread_func(void *arg __maybe_unused)
{
	for (int i = 0; i < 1000; i++) {
		/* Using C11 mtx_t guard */
		guard(mtx)(&c11_mutex);
		c11_shared_counter++;
	}
	return 0;
}

static void test_c11_mutex(void)
{
	printf("Test 1: C11 mtx_t locking\n");

	/* Initialize C11 mutex */
	if (mtx_init(&c11_mutex, mtx_plain) != thrd_success) {
		printf("  Failed to initialize mutex\n");
		return;
	}

	/* The mutex will be automatically unlocked when guard goes out of scope */
	guard(mtx)(&c11_mutex);

	c11_shared_counter++;
	printf("  Counter incremented to: %d\n", c11_shared_counter);

	/* No need to call mtx_unlock() - it happens automatically! */

	/* Destroy mutex when done */
	mtx_destroy(&c11_mutex);
}

static void test_c11_threads(void)
{
	printf("\nTest 2: C11 threads with automatic joining\n");

	/* Initialize mutex */
	if (mtx_init(&c11_mutex, mtx_plain) != thrd_success) {
		printf("  Failed to initialize mutex\n");
		return;
	}

	c11_shared_counter = 0;
	thrd_t threads[5];

	for (int i = 0; i < 5; i++) {
		if (thrd_create(&threads[i], thread_func, NULL) != thrd_success) {
			printf("  Failed to create thread %d\n", i);
			mtx_destroy(&c11_mutex);
			return;
		}
	}

	/* Wait for all threads to complete */
	for (int i = 0; i < 5; i++) {
		thrd_join(threads[i], NULL);
	}

	printf("  Final counter value: %d (expected: 5000)\n", c11_shared_counter);

	mtx_destroy(&c11_mutex);
}

static void test_c11_timed_mutex(void)
{
	printf("\nTest 3: C11 timed mutex\n");

	mtx_t timed_mutex;
	if (mtx_init(&timed_mutex, mtx_timed) != thrd_success) {
		printf("  Failed to initialize timed mutex\n");
		return;
	}

	/* Try to acquire with 1 second timeout */
	CLASS(mtx_timed, lock)(&timed_mutex, 1);
	if (lock) {
		printf("  Acquired timed mutex successfully\n");
		/* Will be automatically unlocked */
	} else {
		printf("  Failed to acquire timed mutex within timeout\n");
	}

	mtx_destroy(&timed_mutex);
}

/* Thread function for join guard test */
static int simple_thread_func(void *arg __maybe_unused)
{
	printf("  Thread running\n");
	return 0;
}

static void test_c11_thread_join_guard(void)
{
	printf("\nTest 4: C11 thread join guard\n");

	thrd_t thread;
	if (thrd_create(&thread, simple_thread_func, NULL) == thrd_success) {
		/* Create a guard that will join the thread when it goes out of scope */
		AUTO_JOIN_THREAD(thread);
		printf("  Thread created with join guard\n");
		/* Thread will be automatically joined when guard goes out of scope */
	}
}

static void test_mixed_primitives(void)
{
	printf("\nTest 5: Mixed pthread and C11 primitives\n");

	/* You can use both pthread and C11 guards in the same program */
	pthread_mutex_t pt_mutex = PTHREAD_MUTEX_INITIALIZER;
	mtx_t c11_mtx;

	if (mtx_init(&c11_mtx, mtx_plain) != thrd_success) {
		printf("  Failed to initialize C11 mutex\n");
		return;
	}

	{
		/* Use pthread guard in its own scope */
		guard(mutex)(&pt_mutex);
		printf("  pthread mutex locked\n");
	} /* pthread mutex automatically unlocked here */

	{
		/* Use C11 guard in its own scope */
		guard(mtx)(&c11_mtx);
		printf("  C11 mtx locked\n");
	} /* C11 mtx automatically unlocked here */

	pthread_mutex_destroy(&pt_mutex);
	mtx_destroy(&c11_mtx);
}

int main(void)
{
	printf("C11 Thread Primitives with Scope-based Guards\n");
	printf("=============================================\n");

	test_c11_mutex();
	test_c11_threads();
	test_c11_timed_mutex();
	test_c11_thread_join_guard();
	test_mixed_primitives();

	printf("\nAll C11 tests completed!\n");
	return 0;
}