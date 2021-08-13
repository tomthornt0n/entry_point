
//~NOTE(tbt): msvc
#if defined(_MSC_VER)
# define Compiler_MSVC 1

//-NOTE(tbt): os detection
# if defined(_WIN32)
#  define OS_Windows 1
# else
#  error Could not detect OS
# endif

//-NOTE(tbt): architecture detection
# if defined(_M_AMD64)
#  define Arch_X64 1
# elif defined(_M_I86)
#  define Arch_X86 1
# elif defined(_M_ARM)
#  define Arch_ARM 1
// TODO(tbt): ARM64 ???
# else
#  error Could not detect architecture
# endif

//~NOTE(tbt): clang
#elif defined(__clang__)
# define Compiler_Clang 1

//-NOTE(tbt): os detection
# if defined(_WIN32)
#  define OS_Windows 1
# elif defined(__gnu_linux__)
#  define OS_Linux 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_Mac 1
# else
#  error Could not detect OS
# endif

//-NOTE(tbt): architecture detection
# if defined(__amd64__)
#  define Arch_X64 1
# elif defined(__i386__)
#  define Arch_X86 1
# elif defined(__arm__)
#  define Arch_ARM 1
# elif defined(__aarch64__)
#  define Arch_ARM64 1
# else
#  error Could not detect architecture
# endif

//~NOTE(tbt): GCC
#elif defined(__GNUC__)
# define Compiler_GCC 1

//-NOTE(tbt): os detection
# if defined(_WIN32)
#  define OS_Windows 1
# elif defined(__gnu_linux__)
#  define OS_Linux 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_Mac 1
# else
#  error Could not detect OS
# endif

//-NOTE(tbt): architecture detection
# if defined(__amd64__)
#  define Arch_X64 1
# elif defined(__i386__)
#  define Arch_X86 1
# elif defined(__arm__)
#  define Arch_ARM 1
# elif defined(__aarch64__)
#  define Arch_ARM64 1
# else
#  error Could not detect architecture
# endif

#else
# error Could not detect compiler

#endif

//~NOTE(tbt): zero fill non set macros

//-NOTE(tbt): compiler
#if !defined(Compiler_MSVC)
# define Compiler_MSVC 0
#endif
#if !defined(Compiler_Clang)
# define Compiler_Clang 0
#endif
#if !defined(Compiler_GCC)
# define Compiler_GCC 0
#endif

//-NOTE(tbt): OS
#if !defined(OS_Windows)
# define OS_Windows 0
#endif
#if !defined(OS_Linux)
# define OS_Linux 0
#endif
#if !defined(OS_Mac)
# define OS_Mac 0
#endif

//-NOTE(tbt): architecture
#if defined(Arch_X64)
# define UseSSE2 1
# define UseSSE3 1 // TODO(tbt): don't just assume all x64 cpus have SSE3 (although this shouldn't really be an problem)
# include <emmintrin.h>
# include <immintrin.h>
#else
# define Arch_X64 0
#endif
#if !defined(Arch_X86)
# define Arch_X86 0
#endif
#if !defined(Arch_ARM)
# define Arch_ARM 0
#endif
#if !defined(Arch_ARM64)
# define Arch_ARM64 0
#endif
#if !defined(UseSSE3)
# define UseSSE3 0
#endif
#if !defined(UseSSE2)
# if UseSSE3
#  define UseSSE2 1
# else
#  define UseSSE2 0
# endif
#endif

//-NOTE(tbt): build options
#if !defined(Build_ModeDebug)
# define Build_ModeDebug 0
#endif
#if !defined(Build_ModeRelease)
# define Build_ModeRelease 0
#endif
#if !defined(Build_NoCRT)
# define Build_NoCRT 0
#endif
#if !defined(Build_EnableAsserts)
# define Build_EnableAsserts 0
#endif
