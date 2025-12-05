#pragma once

// Source - https://stackoverflow.com/a
// Posted by Columbo, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-05, License - CC BY-SA 3.0

#define VARGS_(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N
#define VARGS(...) VARGS_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

#define REQUIRE_CLOSING static_assert(true)