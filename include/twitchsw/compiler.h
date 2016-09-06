#pragma once

#define TSW_COMPILER(COMPILER) (defined(TSW_COMPILER_##COMPILER) && TSW_COMPILER_##COMPILER)
#define TSW_COMPILER_SUPPORTS(FEATURE) (defined(TSW_COMPILER_SUPPORTS_##FEATURE) && TSW_COMPILER_SUPPORTS_##FEATURE)

#ifdef __has_feature
#define TSW_COMPILER_HAS_CLANG_FEATURE(FEATURE) __has_feature(FEATURE)
#else
#define TSW_COMPILER_HAS_CLANG_FEATURE(FEATURE) 0
#endif  // __has_feature

#ifdef __clang__
#define TSW_COMPILER_CLANG 1
#define TSW_COMPILER_SUPPORTS_CXX_REFERENCE_QUALIFIED_FUNCTIONS TSW_COMPILER_HAS_CLANG_FEATURE(cxx_reference_qualified_functions)
#endif  // __clang__

#ifdef _MSC_VER
#define TSW_COMPILER_MSVC
#endif  // _MSC_VER

#ifdef __GNUC__
#define TSW_COMPILER_GCC 1
#endif  // __GNUC__

#if TSW_COMPILER_CLANG || TSW_COMPILER_GCC
#define TSW_COMPILER_CLANG_OR_GCC 1
#define TSW_COMPILER_GCC_OR_CLANG 1
#endif  // TSW_COMPILER_CLANG || TSW_COMPILER_GCC

#if !defined(UNUSED) && TSW_COMPILER(MSVC)
#define UNUSED(VARIABLE) ((void)&VARIABLE)
#endif  // !defined(UNUSED) && TSW_COMPILER(MSVC)

#if !defined(UNUSED)
#define UNUSED(VARIABLE) ((void)(VARIABLE))
#endif  // !defined(UNUSED)


// Branch prediction
#if !defined(LIKELY) && TSW_COMPILER(CLANG_OR_GCC)
#define LIKELY(CONDITION) __builtin_expect(!!(CONDITION), 1)
#endif  // !defined(LIKELY) TSW_COMPILER(CLANG_OR_GCC)

#if !defined(LIKELY)
#define LIKELY(CONDITION) CONDITION
#endif  // !defined(LIKELY)

#if !defined(UNLIKELY) && TSW_COMPILER(CLANG_OR_GCC)
#define UNLIKELY(CONDITION) __builtin_expect(!!(CONDITION), 0)
#endif  // !defined(LIKELY) TSW_COMPILER(CLANG_OR_GCC)

#if !defined(UNLIKELY)
#define UNLIKELY(CONDITION) CONDITION
#endif  // !defined(LIKELY)

// Always inline in release builds
#if !defined(ALWAYS_INLINE) && TSW_COMPILER(CLANG_OR_GCC) && defined(NDEBUG) && !TSW_COMPILER(MINGW)
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#endif  // !defined(ALWAYS_INLINE) && TSW_COMPILER(CLANG_OR_GCC) && defined(NDEBUG) && !TSW_COMPILER(MINGW)

#if !defined(ALWAYS_INLINE) && TSW_COMPILER(MSVC) && defined(NDEBUG)
#define ALWAYS_INLINE __forceinline
#endif  // !defined(ALWAYS_INLINE) && TSW_COMPILER(MSVC) && defined(NDEBUG)

#if !defined(ALWAYS_INLINE)
#define ALWAYS_INLINE inline
#endif  // !defined(ALWAYS_INLINE)

// Never inline
#if !defined(ALWAYS_INLINE) && TSW_COMPILER(CLANG_OR_GCC)
#define NEVER_INLINE __attribute__((__noinline__))
#endif  // !defined(ALWAYS_INLINE) && TSW_COMPILER(CLANG_OR_GCC)

#if !defined(NEVER_INLINE) && TSW_COMPILER(MSVC)
#define NEVER_INLINE __declspec(noinline)
#endif  // !defined(NEVER_INLINE) && TSW_COMPILER(MSVC)

#if !defined(NEVER_INLINE)
#define NEVER_INLINE
#endif  // !defined(NEVER_INLINE)
