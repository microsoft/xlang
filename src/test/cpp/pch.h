#pragma once

#include "catch.hpp"
#include "winrt/base.h"

template<bool B> bool static_require()
{
  return B;
}

#define STATIC_REQUIRE(...) REQUIRE(static_require<__VA_ARGS__>())
