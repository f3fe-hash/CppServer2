#pragma once

/* Attributes. */
#define _throw      __attribute__((nothrow))
#define _wur        __attribute__((warn_unused_result))
#define _pure       __attribute__((pure))
#define _const      __attribute__((const))
#define _unused     __attribute__((unused))
#define _nnull(...) __attribute__((nonnull (__VA_ARGS__)))

/* Branch prediction. */
#define _likely(x)    __builtin_expect(!!(x), 1)
#define _unlikely(x)  __builtin_expect(!!(x), 0)
#define _expect(x, y) __builtin_expect(x, y)

/* Cache. */
#define _prefetch(ptr, rw, loc) __builtin_prefetch(ptr, rw, loc)

/* Use endline instead of std::endl,
   because std::endl force-flushes
   data, which is not necessary. */
#define endline "\n"

#include <stdint.h>
#include <stdbool.h>
