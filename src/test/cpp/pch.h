#pragma once

#include "catch.hpp"
#include "winrt/base.h"
using namespace std::literals;

template<bool B> bool static_require()
{
    return B;
}

#define STATIC_REQUIRE(...) { constexpr bool require = __VA_ARGS__; REQUIRE(static_require<require>()); }
