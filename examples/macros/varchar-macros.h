#ifndef VARCHAR_MACROS_H
#define VARCHAR_MACROS_H

#include <string.h>
#include <ctype.h>

/*
 * Notes:
 * - Assumes `v` is a VARCHAR struct (not a pointer).
 * - Ensures v.arr is always nul-terminated at position [v.len] if possible.
 * - All string-setting macros force nul termination if space allows.
 * - All read/write operations treat arr as raw bytes, not C strings.
 */

/* Clear to empty */
#define VCLR(v) do { \
    (v).len = 0; \
    memset((v).arr, 0, sizeof((v).arr)); \
} while (0)

/* Set from C string (truncate if needed) */
#define VSET(v, cstr) do { \
    size_t __n = strlen(cstr); \
    size_t __cap = sizeof((v).arr); \
    if (__n >= __cap) __n = __cap - 1; \
    memcpy((v).arr, (cstr), __n); \
    (v).arr[__n] = '\0'; \
    (v).len = (unsigned short)__n; \
} while (0)

/* Null-terminate into a separate buffer */
#define VTOC(v, buf) do { \
    size_t __n = ((v).len < sizeof(buf)-1) ? (v).len : (sizeof(buf)-1); \
    memcpy((buf), (v).arr, __n); \
    (buf)[__n] = '\0'; \
} while (0)

/* Get length */
#define VLEN(v) ((v).len)

/* Append another VARCHAR (truncate if needed) */
#define VAPPEND(v1, v2) do { \
    size_t __max = sizeof((v1).arr); \
    size_t __free = (__max > (size_t)(v1).len + 1) ? (__max - (v1).len - 1) : 0; \
    size_t __n = ((v2).len < __free) ? (v2).len : __free; \
    memcpy((v1).arr + (v1).len, (v2).arr, __n); \
    (v1).len += (unsigned short)__n; \
    (v1).arr[(v1).len] = '\0'; \
} while (0)

/* Compare (case sensitive) */
#define VCMP(v1, v2) (memcmp((v1).arr, (v2).arr, ((v1).len < (v2).len ? (v1).len : (v2).len)) ?: ((int)(v1).len - (int)(v2).len))

/* Case insensitive compare using function */
#define VICMP(v1, v2) varchar_icmp(&(v1), &(v2))

/* Uppercase in place */
#define VUPPER(v) do { \
    for (size_t __i = 0; __i < (v).len; __i++) \
        (v).arr[__i] = (unsigned char)toupper((v).arr[__i]); \
    if ((v).len < sizeof((v).arr)) (v).arr[(v).len] = '\0'; \
} while (0)

/* Lowercase in place */
#define VLOWER(v) do { \
    for (size_t __i = 0; __i < (v).len; __i++) \
        (v).arr[__i] = (unsigned char)tolower((v).arr[__i]); \
    if ((v).len < sizeof((v).arr)) (v).arr[(v).len] = '\0'; \
} while (0)

/* Trim both ends in-place */
#define VTRIM(v) do { \
    int __s = 0, __e = (v).len - 1; \
    while (__s <= __e && isspace((unsigned char)(v).arr[__s])) __s++; \
    while (__e >= __s && isspace((unsigned char)(v).arr[__e])) __e--; \
    size_t __n = (__e >= __s) ? (__e - __s + 1) : 0; \
    if (__s > 0 && __n > 0) memmove((v).arr, (v).arr + __s, __n); \
    (v).len = (unsigned short)__n; \
    if ((v).len < sizeof((v).arr)) (v).arr[(v).len] = '\0'; \
} while (0)

/* Fill with single character */
#define VFILL(v, ch, n) do { \
    size_t __n = ((n) > sizeof((v).arr) - 1) ? sizeof((v).arr) - 1 : (n); \
    memset((v).arr, (ch), __n); \
    (v).len = (unsigned short)__n; \
    (v).arr[__n] = '\0'; \
} while (0)

/* Copy one VARCHAR to another */
#define VCOPY(dest, src) do { \
    size_t __n = ((src).len < sizeof((dest).arr) - 1) ? (src).len : sizeof((dest).arr) - 1; \
    memcpy((dest).arr, (src).arr, __n); \
    (dest).arr[__n] = '\0'; \
    (dest).len = (unsigned short)__n; \
} while (0)

/* Zero out arr but preserve .len (optional) */
#define VZERO(v) do { \
    memset((v).arr, 0, sizeof((v).arr)); \
} while (0)

#endif /* VARCHAR_MACROS_H */
