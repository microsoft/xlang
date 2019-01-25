
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

#if __has_include(<WindowsNumerics.impl.h>)
#define WINRT_NUMERICS
#include <directxmath.h>
#define _WINDOWS_NUMERICS_NAMESPACE_ winrt::Windows::Foundation::Numerics
#define _WINDOWS_NUMERICS_BEGIN_NAMESPACE_ namespace winrt::Windows::Foundation::Numerics
#define _WINDOWS_NUMERICS_END_NAMESPACE_
#ifdef __clang__
#define _XM_NO_INTRINSICS_
#endif
#include <WindowsNumerics.impl.h>
#ifdef __clang__
#undef _XM_NO_INTRINSICS_
#endif
#undef _WINDOWS_NUMERICS_NAMESPACE_
#undef _WINDOWS_NUMERICS_BEGIN_NAMESPACE_
#undef _WINDOWS_NUMERICS_END_NAMESPACE_
#endif
