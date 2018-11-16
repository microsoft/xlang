
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <clocale>
#include <cstddef>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <vector>
#include <experimental/coroutine>

#include <intrin.h>

#ifndef WINRT_EXPORT
#define WINRT_EXPORT
#else
export module winrt;
#endif
