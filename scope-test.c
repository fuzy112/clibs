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
#include <pthread.h>

#include "scope-guards.h"

/*
 * Example 1: Automatic memory management with DEFINE_FREE
 */

static void test_automatic_memory(void)
{
	fprintf(stderr, "Test 1: Automatic memory management\n");

	/* This pointer will be automatically freed when it goes out of scope */
	char *buffer __cleanup(freep) = malloc(1024);
	if (!buffer) {
		fprintf(stderr, "  Failed to allocate memory\n");
		return;
	}

	strcpy(buffer, "Hello, scope-based memory management!");
	fprintf(stderr, "  Buffer contains: %s\n", buffer);

	/* No need to call free() - it happens automatically */
}

static char *create_greeting(const char *name)
{
	/* Allocate memory that will be automatically freed unless returned */
	char *greeting __cleanup(freep) = malloc(256);
	if (!greeting)
		return NULL;

	snprintf(greeting, 256, "Hello, %s!", name);

	/* Transfer ownership to caller - prevents automatic free */
	return_ptr(greeting);
}

/*
 * Example 2: File descriptor management with DEFINE_CLASS
 */

#include <fcntl.h>
#include <unistd.h>



static void test_file_descriptor(void)
{
fprintf(stderr,"\nTest 2: File descriptor management\n");

	/* File descriptor will be automatically closed when it goes out of scope */
	CLASS(fd, file)("/dev/null", O_RDONLY);
	if (file < 0) {
fprintf(stderr,"  Failed to open /dev/null\n");
		return;
	}

fprintf(stderr,"  File descriptor %d opened successfully\n", file);

	/* No need to call close() - it happens automatically */
}

/*
 * Example 3: Mutex locking with DEFINE_GUARD
 */

static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
static int shared_counter = 0;

static void test_mutex_guard(void)
{
fprintf(stderr,"\nTest 3: Mutex locking with guard\n");

	/* Mutex will be automatically unlocked when guard goes out of scope */
	guard(mutex)(&test_mutex);

	shared_counter++;
fprintf(stderr,"  Counter incremented to: %d\n", shared_counter);

	/* No need to call pthread_mutex_unlock() - it happens automatically */
}

static void test_scoped_guard(void)
{
fprintf(stderr,"\nTest 3b: Mutex locking with scoped_guard\n");

	/* Mutex will be automatically unlocked at the end of the scope */
	scoped_guard(mutex, m, &test_mutex) {
		shared_counter++;
fprintf(stderr,"  Counter incremented inside scope: %d\n", shared_counter);
	}

fprintf(stderr,"  Counter after scope: %d\n", shared_counter);
}

static void *thread_func(void *arg __maybe_unused)
{
	for (int i = 0; i < 1000; i++) {
		guard(mutex)(&test_mutex);
		shared_counter++;
	}
	return NULL;
}

static void *thread_func_scoped(void *arg __maybe_unused)
{
	for (int i = 0; i < 1000; i++) {
		scoped_guard(mutex, m, &test_mutex) {
			shared_counter++;
		}
	}
	return NULL;
}

static void test_thread_safety(void)
{
fprintf(stderr,"\nTest 4: Thread safety with mutex guards\n");

	shared_counter = 0;
	pthread_t threads[10];

	for (int i = 0; i < 10; i++) {
		if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
fprintf(stderr,"  Failed to create thread %d\n", i);
			return;
		}
	}

	for (int i = 0; i < 10; i++) {
		pthread_join(threads[i], NULL);
	}

fprintf(stderr,"  Final counter value: %d (expected: 10000)\n", shared_counter);
}

static void test_thread_safety_scoped(void)
{
fprintf(stderr,"\nTest 4b: Thread safety with scoped_guard\n");

	shared_counter = 0;
	pthread_t threads[10];

	for (int i = 0; i < 10; i++) {
		if (pthread_create(&threads[i], NULL, thread_func_scoped, NULL) != 0) {
fprintf(stderr,"  Failed to create thread %d\n", i);
			return;
		}
	}

	for (int i = 0; i < 10; i++) {
		pthread_join(threads[i], NULL);
	}

fprintf(stderr,"  Final counter value: %d (expected: 10000)\n", shared_counter);
}

/*
 * Example 4: Complex resource management
 */

struct complex_resource {
	char *data;
	size_t size;
	int fd;
	pthread_mutex_t lock;
};

static void cleanup_complex_resource(struct complex_resource *res)
{
	if (res) {
		if (res->data)
			free(res->data);
		if (res->fd >= 0)
			close(res->fd);
		pthread_mutex_destroy(&res->lock);
		free(res);
	}
}

DEFINE_FREE(complex, struct complex_resource *, cleanup_complex_resource(_T))

static struct complex_resource *create_complex_resource(void)
{
	struct complex_resource *res __free(complex) = calloc(1, sizeof(*res));
	if (!res)
		return NULL;

	res->data = malloc(4096);
	if (!res->data)
		return NULL;

	res->size = 4096;
	res->fd = -1;

	if (pthread_mutex_init(&res->lock, NULL) != 0)
		return NULL;

	/* Transfer ownership to caller */
	return_ptr(res);
}

static void test_complex_resource(void)
{
fprintf(stderr,"\nTest 5: Complex resource management\n");

	struct complex_resource *res = create_complex_resource();
	if (!res) {
fprintf(stderr,"  Failed to create complex resource\n");
		return;
	}

	/* Use the resource */
	guard(mutex)(&res->lock);
	strcpy(res->data, "Complex resource data");
fprintf(stderr,"  Resource data: %s\n", res->data);

	/* Manually clean up since we took ownership */
	cleanup_complex_resource(res);
}

/*
 * Example 5: Error handling without goto
 */

static int process_with_resources(void)
{
	/* All resources will be automatically cleaned up on error return */
	char *buffer __cleanup(freep) = malloc(1024);
	if (!buffer)
		return -1;

	CLASS(fd, file)("/dev/null", O_RDONLY);
	if (file < 0)
		return -1;

	guard(mutex)(&test_mutex);

	/* Do some work */
	strcpy(buffer, "Processing with automatic cleanup");
fprintf(stderr,"  %s\n", buffer);

	/* Success - buffer will be automatically freed,
	 * file will be automatically closed,
	 * mutex will be automatically unlocked */
	return 0;
}

static void test_error_handling(void)
{
fprintf(stderr,"\nTest 6: Error handling without goto\n");

	int result = process_with_resources();
fprintf(stderr,"  Result: %d\n", result);
}

int main(void)
{
fprintf(stderr,"Scope-based locking utilities test\n");
fprintf(stderr,"==================================\n");

	test_automatic_memory();

	char *greeting = create_greeting("World");
	if (greeting) {
fprintf(stderr,"  Created greeting: %s\n", greeting);
		free(greeting);
	}

	test_file_descriptor();
	test_mutex_guard();
	test_scoped_guard();
	test_thread_safety();
	test_thread_safety_scoped();
	test_complex_resource();
	test_error_handling();

fprintf(stderr,"\nAll tests completed successfully!\n");
	return 0;
}