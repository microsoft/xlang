#pragma once

#include <pal.h>
#include <new>
#include <algorithm>

#ifdef _DEBUG

#if XLANG_COMPILER_MSVC
#define XLANG_ASSERT _ASSERTE
#else
#include <cassert>
#define XLANG_ASSERT assert
#endif
#define XLANG_VERIFY XLANG_ASSERT
#define XLANG_VERIFY_(result, expression) XLANG_ASSERT(result == expression)

#else

#define XLANG_ASSERT(expression) ((void)0)
#define XLANG_VERIFY(expression) (void)(expression)
#define XLANG_VERIFY_(result, expression) (void)(expression)

#endif

inline bool operator==(xlang_guid const& lhs, xlang_guid const& rhs) noexcept
{
    static_assert(sizeof(xlang_guid) % sizeof(size_t) == 0);
    constexpr size_t count = sizeof(xlang_guid) / sizeof(size_t);

    auto guid1 = reinterpret_cast<size_t const*>(&lhs);
    auto guid2 = reinterpret_cast<size_t const*>(&rhs);
    return std::equal(guid1, guid1 + count, guid2, guid2 + count);
}