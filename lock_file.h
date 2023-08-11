#ifndef LOCK_FILE_H
#define LOCK_FILE_H

#define LOCK_FILE_INIT -1

int lock_file (int *lock, const char *filename);

int try_lock_file (int *lock, const char *filename);

void unlock_file (int *lock);

#endif // LOCK_FILE_H
