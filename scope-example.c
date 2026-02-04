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
#include <pthread.h>

#include "scope-guards.h"

/*
 * Simple example demonstrating scope-based locking utilities
 */

static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Example 1: Basic memory management */
static void example_memory(void)
{
	printf("Example 1: Automatic memory management\n");

	/* Method 1: Using __cleanup(freep) directly */
	char *buffer __cleanup(freep) = malloc(100);
	if (!buffer) {
		printf("  Allocation failed\n");
		return;
	}

	snprintf(buffer, 100, "Automatically managed memory");
	printf("  Method 1: %s\n", buffer);

	/* Method 2: Using __cleanup directly */
	char *buffer2 __cleanup(freep) = malloc(100);
	if (buffer2) {
		snprintf(buffer2, 100, "Using __cleanup(freep) directly");
		printf("  Method 2: %s\n", buffer2);
	}

	/* No need to call free() - it happens automatically! */
}

/* Example 2: Basic locking */
static void example_locking(void)
{
	printf("\nExample 2: Automatic mutex locking\n");

	/* Method 1: Using guard() directly */
	{
		guard(mutex)(&global_mutex);
		printf("  Method 1: Using guard() - mutex is locked\n");
		printf("  Doing some work...\n");
		/* Mutex automatically unlocked here */
	}

	/* Method 2: Using AUTO_MUTEX convenience macro */
	{
		AUTO_MUTEX(&global_mutex);
		printf("  Method 2: Using AUTO_MUTEX - mutex is locked\n");
		printf("  Doing some work...\n");
		/* Mutex automatically unlocked here */
	}

	/* Method 3: Using scoped_guard() */
	scoped_guard(mutex, m, &global_mutex) {
		printf("  Method 3: Using scoped_guard() - mutex is locked\n");
		printf("  Doing some work...\n");
		/* Mutex automatically unlocked at end of scope */
	}

	/* No need to call pthread_mutex_unlock() - it happens automatically! */
}

/* Example 3: Error handling without goto */
static int example_error_handling(int should_fail)
{
	printf("\nExample 3: Error handling without goto\n");

	/* All resources declared here will be automatically cleaned up
	 * if we return early due to an error */
	char *data __cleanup(freep) = malloc(512);
	if (!data) {
		printf("  Failed to allocate data\n");
		return -1;
	}

	guard(mutex)(&global_mutex);

	if (should_fail) {
		printf("  Simulating failure - returning early\n");
		return -1;  /* Both data and mutex are automatically cleaned up! */
	}

	snprintf(data, 512, "Successfully processed data");
	printf("  %s\n", data);

	return 0;  /* Resources are still automatically cleaned up */
}

/* Example 4: Returning allocated memory */
static char *example_return_memory(void)
{
	printf("\nExample 4: Returning allocated memory\n");

	/* This memory would normally be automatically freed */
	char *result __cleanup(freep) = malloc(256);
	if (!result)
		return NULL;

	snprintf(result, 256, "This memory is being returned to caller");

	/* Use return_ptr to transfer ownership to caller */
	return_ptr(result);
}

int main(void)
{
	printf("Scope-based Locking Utilities - Simple Examples\n");
	printf("===============================================\n");

	example_memory();
	example_locking();

	example_error_handling(0);
	example_error_handling(1);

	char *returned = example_return_memory();
	if (returned) {
		printf("  Returned: %s\n", returned);
		free(returned);  /* Caller must free since we transferred ownership */
	}

	printf("\nAll examples completed!\n");
	printf("Note: No manual free() or pthread_mutex_unlock() calls needed!\n");

	return 0;
}