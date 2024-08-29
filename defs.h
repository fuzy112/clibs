/* Copyright © 2024  Zhengyi Fu <i@fuzy.me> */

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

#ifndef DEFS_H
#define DEFS_H

#ifdef __cplusplus
/* clang-format off */
#define C_DECL extern "C"
#define C_DECL_BEGIN extern "C" {
#define C_DECL_END }
/* clang-format on */
#else
#define C_DECL extern
#define C_DECL_BEGIN
#define C_DECL_END
#endif

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alias-function-attribute
 */
#define __alias(symbol)                 __attribute__((__alias__(#symbol)))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-aligned-function-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-aligned-type-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-aligned-variable-attribute
 */
#define __aligned(x)                    __attribute__((__aligned__(x)))
#define __aligned_largest               __attribute__((__aligned__))

/*
 * Note: do not use this directly. Instead, use __alloc_size() since it is conditionally
 * available and includes other attributes. For GCC < 9.1, __alloc_size__ gets undefined
 * in compiler-gcc.h, due to misbehaviors.
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005fsize-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#alloc-size
 */
#define __alloc_size__(x, ...)		__attribute__((__alloc_size__(x, ## __VA_ARGS__)))

/*
 * Note: users of __always_inline currently do not write "inline" themselves,
 * which seems to be required by gcc to apply the attribute according
 * to its docs (and also "warning: always_inline function might not be
 * inlinable [-Wattributes]" is emitted).
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-always_005finline-function-attribute
 * clang: mentioned
 */
#ifndef __always_inline
# define __always_inline                 inline __attribute__((__always_inline__))
#endif

/*
 * The second argument is optional (default 0), so we use a variadic macro
 * to make the shorthand.
 *
 * Beware: Do not apply this to functions which may return
 * ERR_PTRs. Also, it is probably unwise to apply it to functions
 * returning extra information in the low bits (but in that case the
 * compiler should see some alignment anyway, when the return value is
 * massaged by 'flags = ptr & 3; ptr &= ~3;').
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-assume_005faligned-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#assume-aligned
 */
#define __assume_aligned(a, ...)        __attribute__((__assume_aligned__(a, ## __VA_ARGS__)))

/*
 * The second argument is optional (default 0), so we use a variadic macro
 * to make the shorthand.
 *
 * Beware: Do not apply this to functions which may return
 * ERR_PTRs. Also, it is probably unwise to apply it to functions
 * returning extra information in the low bits (but in that case the
 * compiler should see some alignment anyway, when the return value is
 * massaged by 'flags = ptr & 3; ptr &= ~3;').
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-assume_005faligned-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#assume-aligned
 */
#define __assume_aligned(a, ...)        __attribute__((__assume_aligned__(a, ## __VA_ARGS__)))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-cleanup-variable-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#cleanup
 */
#define __cleanup(func)			__attribute__((__cleanup__(func)))

/*
 * Note the long name.
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-const-function-attribute
 */
#ifndef __attribute_const__
# define __attribute_const__             __attribute__((__const__))
#endif

/*
 * Optional: only supported since gcc >= 14
 * Optional: only supported since clang >= 18
 *
 *   gcc: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108896
 * clang: https://reviews.llvm.org/D148381
 */
#if __has_attribute(__counted_by__)
# define __counted_by(member)		__attribute__((__counted_by__(member)))
#else
# define __counted_by(member)
#endif

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-malloc-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#malloc
 */
#define __malloc                        __attribute__((__malloc__))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-mode-type-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-mode-variable-attribute
 */
#define __mode(x)                       __attribute__((__mode__(x)))

/*
 * Optional: only supported since gcc >= 7
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/x86-Function-Attributes.html#index-no_005fcaller_005fsaved_005fregisters-function-attribute_002c-x86
 * clang: https://clang.llvm.org/docs/AttributeReference.html#no-caller-saved-registers
 */
#if __has_attribute(__no_caller_saved_registers__)
# define __no_caller_saved_registers	__attribute__((__no_caller_saved_registers__))
#else
# define __no_caller_saved_registers
#endif

/*
 * Add the pseudo keyword 'fallthrough' so case statement blocks
 * must end with any of these keywords:
 *   break;
 *   fallthrough;
 *   continue;
 *   goto <label>;
 *   return [expression];
 *
 *  gcc: https://gcc.gnu.org/onlinedocs/gcc/Statement-Attributes.html#Statement-Attributes
 */
#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif

/*
 * gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes
 * clang: https://clang.llvm.org/docs/AttributeReference.html#flatten
 */
# define __flatten			__attribute__((flatten))

/*
 * Note the missing underscores.
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-noinline-function-attribute
 * clang: mentioned
 */
#define   noinline                      __attribute__((__noinline__))

/*
 * Optional: only supported since gcc >= 8
 * Optional: not supported by clang
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-nonstring-variable-attribute
 */
#if __has_attribute(__nonstring__)
# define __nonstring                    __attribute__((__nonstring__))
#else
# define __nonstring
#endif

/*
 * Optional: only supported since GCC >= 7.1, clang >= 13.0.
 *
 *      gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-no_005fprofile_005finstrument_005ffunction-function-attribute
 *    clang: https://clang.llvm.org/docs/AttributeReference.html#no-profile-instrument-function
 */
#if __has_attribute(__no_profile_instrument_function__)
# define __no_profile                  __attribute__((__no_profile_instrument_function__))
#else
# define __no_profile
#endif

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-noreturn-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#noreturn
 * clang: https://clang.llvm.org/docs/AttributeReference.html#id1
 */
#define __noreturn                      __attribute__((__noreturn__))

/*
 * Optional: only supported since GCC >= 11.1, clang >= 7.0.
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-no_005fstack_005fprotector-function-attribute
 *   clang: https://clang.llvm.org/docs/AttributeReference.html#no-stack-protector-safebuffers
 */
#if __has_attribute(__no_stack_protector__)
# define __no_stack_protector		__attribute__((__no_stack_protector__))
#else
# define __no_stack_protector
#endif

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-packed-type-attribute
 * clang: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-packed-variable-attribute
 */
#define __packed                        __attribute__((__packed__))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-pure-function-attribute
 */
#define __pure                          __attribute__((__pure__))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-section-function-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-section-variable-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#section-declspec-allocate
 */
#define __section(section)              __attribute__((__section__(section)))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-unused-function-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-unused-type-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-unused-variable-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Label-Attributes.html#index-unused-label-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#maybe-unused-unused
 */
#define __always_unused                 __attribute__((__unused__))
#define __maybe_unused                  __attribute__((__unused__))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-used-function-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-used-variable-attribute
 */
#define __used                          __attribute__((__used__))

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-warn_005funused_005fresult-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#nodiscard-warn-unused-result
 */
#define __must_check                    __attribute__((__warn_unused_result__))

/*
 * Optional: only supported since clang >= 14.0
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-warning-function-attribute
 */
#if __has_attribute(__warning__)
# define __compiletime_warning(msg)     __attribute__((__warning__(msg)))
#else
# define __compiletime_warning(msg)
#endif

/*
 * Optional: only supported since clang >= 14.0
 *
 * clang: https://clang.llvm.org/docs/AttributeReference.html#disable-sanitizer-instrumentation
 *
 * disable_sanitizer_instrumentation is not always similar to
 * no_sanitize((<sanitizer-name>)): the latter may still let specific sanitizers
 * insert code into functions to prevent false positives. Unlike that,
 * disable_sanitizer_instrumentation prevents all kinds of instrumentation to
 * functions with the attribute.
 */
#if __has_attribute(disable_sanitizer_instrumentation)
# define __disable_sanitizer_instrumentation \
	 __attribute__((disable_sanitizer_instrumentation))
#else
# define __disable_sanitizer_instrumentation
#endif

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-weak-function-attribute
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-weak-variable-attribute
 */
#define __weak                          __attribute__((__weak__))

/*
 * Used by functions that use '__builtin_return_address'. These function
 * don't want to be splited or made inline, which can make
 * the '__builtin_return_address' get unexpected address.
 */
#define __fix_address noinline __noclone

#endif /*! DEFS_H */
