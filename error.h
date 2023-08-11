#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

#define return_val_err(val, err) return ((errno = (err)), (val))

#define return_null_err(err) return_val_err ((void *)NULL, err)

#define return_fail_err(err) return_val_err (-1, err)

#define return_err(err) return_val_err (err, err)

#define return_neg_err(err) return_val_err (-err, err)

#endif // ERROR_H
