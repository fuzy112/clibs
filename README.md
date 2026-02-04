# clibs

Useful data structures and utility in C.

Currently the following constructs are implemented:

- AVL tree
- red-black tree
- base64 encoding and decoding
- circular buffer
- `container_of` macro
- URL encoding and decoding
- hash table
- linked list
- lock file
- radix tree (xarray)
- scope-based resource management (scope.h)

## Scope-based Resource Management

The `scope.h` header provides scope-based resource management utilities inspired by the Linux kernel's implementation (see [LWN article](https://lwn.net/Articles/934679/)). These utilities use the `__attribute__((__cleanup__))` compiler extension to automatically clean up resources when they go out of scope.

### Key Features

1. **Automatic memory management** - Pointers can be automatically freed
2. **Automatic locking** - Mutexes automatically unlocked
3. **Automatic resource cleanup** - File descriptors, etc.
4. **Error handling without goto** - Cleanup happens automatically on early returns
5. **Scoped resource management** - Explicit scope-based guards with `scoped_guard()`

### Basic Usage

```c
#include "scope.h"

/* Define a mutex guard */
DEFINE_GUARD(mutex, pthread_mutex_t *, pthread_mutex_lock(_T), pthread_mutex_unlock(_T))

void example(void) {
    /* Automatic memory management using freep */
    char *buffer __cleanup(freep) = malloc(100);

    /* Automatic mutex locking */
    guard(mutex)(&my_mutex);

    /* No manual cleanup needed! */
}
```

### Scoped Guards

For more explicit scope-based resource management, `scope.h` provides the `scoped_guard()` macro. Unlike `guard()` which acquires a resource for the entire function scope, `scoped_guard()` creates a named variable and restricts the resource lifetime to a specific block.

```c
#include "scope.h"

/* Define a file descriptor guard */
DEFINE_GUARD(fd, int, (_T = open(__VA_ARGS__)), close(_T))

void example(void) {
    /* Scoped file descriptor - automatically closed at block exit */
    scoped_guard(fd, f, "/dev/null", O_RDONLY) {
        if (f >= 0) {
            /* Use file descriptor 'f' within this scope */
            read(f, buffer, sizeof(buffer));
        }
    }
    /* File descriptor is automatically closed here */

    /* Multiple scoped guards can be nested */
    scoped_guard(mutex, m, &mutex) {
        /* Critical section - mutex locked */
        counter++;
    }
    /* Mutex automatically unlocked here */
}
```

Key benefits of `scoped_guard()`:
- **Explicit scope boundaries** - Resource lifetime matches lexical scope
- **Named variables** - Access resource via user-defined variable name
- **Thread safety** - Works correctly in multi-threaded environments
- **Variadic constructors** - Supports guards requiring multiple constructor arguments

### Convenience Headers

For common use cases, `scope-guards.h` provides pre-defined guards:

```c
#include "scope-guards.h"

/* Pre-defined guards for: */
/* - pthread_mutex_t, pthread_rwlock_t, pthread_spinlock_t */
/* - C11 mtx_t, cnd_t, thrd_t */
/* - malloc/free, file descriptors, FILE* */

void example(void) {
    /* Using convenience macros */
    AUTO_MUTEX(&mutex);           /* pthread mutex */
    AUTO_MTX(&c11_mutex);         /* C11 mutex */
    char *ptr __cleanup(freep) = malloc(100); /* Automatic free */
    AUTO_FD("/dev/null", O_RDONLY);      /* Automatic close */
}
```

### C11 Thread Support

The library includes guards for C11 thread primitives:

```c
#include "scope-guards.h"
#include <threads.h>

mtx_t mutex;
cnd_t cond;
thrd_t thread;

void example(void) {
    /* C11 mutex with automatic unlock */
    guard(mtx)(&mutex);

    /* C11 condition variable guard */
    guard(cnd)(&cond);

    /* Thread join guard */
    AUTO_JOIN_THREAD(thread);
}
```

### Examples

- `scope-test.c` - Comprehensive test suite (pthread) including `scoped_guard()` tests
- `scope-example.c` - Simple usage examples
- `scope-c11-test.c` - C11 thread primitives test

### Building

```bash
make scope-test      # Build comprehensive pthread test
make scope-example   # Build simple examples
make scope-c11-test  # Build C11 thread primitives test
```
