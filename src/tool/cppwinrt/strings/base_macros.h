
#ifdef _DEBUG

#define WINRT_ASSERT _ASSERTE
#define WINRT_VERIFY WINRT_ASSERT
#define WINRT_VERIFY_(result, expression) WINRT_ASSERT(result == expression)

#else

#define WINRT_ASSERT(expression) ((void)0)
#define WINRT_VERIFY(expression) (void)(expression)
#define WINRT_VERIFY_(result, expression) (void)(expression)

#endif

#if defined(_MSC_VER)
#define WINRT_EBO __declspec(empty_bases)
#define WINRT_NOVTABLE __declspec(novtable)
#define WINRT_CALL __stdcall
#define WINRT_NOINLINE  __declspec(noinline)
#else
#define WINRT_EBO
#define WINRT_NOVTABLE
#define WINRT_CALL
#define WINRT_NOINLINE
#endif

#define WINRT_SHIM(...) (*(abi_t<__VA_ARGS__>**)&static_cast<__VA_ARGS__ const&>(static_cast<D const&>(*this)))

#ifndef WINRT_EXTERNAL_CATCH_CLAUSE
#define WINRT_EXTERNAL_CATCH_CLAUSE
#endif

#if defined(_MSC_VER)
#ifdef _M_HYBRID
#define WINRT_LINK(function, count) __pragma(comment(linker, "/alternatename:#WINRT_" #function "@" #count "=#" #function "@" #count))
#elif _M_IX86
#define WINRT_LINK(function, count) __pragma(comment(linker, "/alternatename:_WINRT_" #function "@" #count "=_" #function "@" #count))
#else
#define WINRT_LINK(function, count) __pragma(comment(linker, "/alternatename:WINRT_" #function "=" #function))
#endif
#else
#define WINRT_LINK(function, count)
#endif

#if defined _M_ARM
#define WINRT_INTERLOCKED_READ_MEMORY_BARRIER (__dmb(_ARM_BARRIER_ISH));
#elif defined _M_ARM64
#define WINRT_INTERLOCKED_READ_MEMORY_BARRIER (__dmb(_ARM64_BARRIER_ISH));
#endif

#define WINRT_IMPL_STRING_1(expression) #expression
#define WINRT_IMPL_STRING(expression) WINRT_IMPL_STRING_1(expression)

// Note: this is a workaround for a false-positive warning produced by the Visual C++ 15.9 compiler.
#pragma warning(disable : 5046)
