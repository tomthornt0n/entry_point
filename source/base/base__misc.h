
//~NOTE(tbt): avoid ambiguous 'static' keyword

// NOTE(tbt): these could theoritically be inverted to allow
//            for building more traditionally, e.g. 1 TU per file

// NOTE(tbt): WARING - these are #undef'ed and then re-#define'd when
//                     including <windows.h> to avoid conflicts. Only changing
//                     them here might not do what you think

#define Function static
#define Global   static
#define Persist  static

//~NOTE(tbt): preprocessor helpers
#define Glue_(A, B) A ## B
#define Glue(A, B) Glue_(A, B)

#define Stringify_(A) #A
#define Stringify(A) Stringify_(A)
#define WidenString(S) Glue_(L, S)
#define WideStringify(A) WidenString(Stringify(A))

//~NOTE(tbt): size helpers
#define Bytes(N)      Glue_(N, LLU)
#define Kilobytes(N)  (1000LLU * (uint64_t)Bytes(N))
#define Megabytes(N)  (1000LLU * (uint64_t)Kilobytes(N))
#define Gigabytes(N)  (1000LLU * (uint64_t)Megabytes(N))
#define Terabytes(N)  (1000LLU * (uint64_t)Gigabytes(N))

//~NOTE(tbt): useful constants
#define Pi (3.1415926535897f)

//~NOTE(tbt): misc helpers
#define ArrayCount(A) (sizeof(A) / sizeof((A)[0]))
#define Bit(N) (1 << N)

#define ApplicationNameString Stringify(ApplicationName)
#define ApplicationNameWString WideStringify(ApplicationName)

#define DeferLoop(BEGIN, END) for(Bool _i = ((BEGIN), True); True == _i; ((END), _i = False))

#define Eq(A, B) (sizeof(A) == sizeof(B) && M_Compare(&(A), &(B), sizeof(A)))

Function int
WrapToBounds(int a, int min, int max)
{
    int result = Abs1I(a);
    int range = max - min;
    result = min + ((result + range) % range);
    return result;
}

#define case(L) case L: Glue(case_, L)

// NOTE(tbt): not sure about this
#define enum(N, T) typedef T N; typedef enum N N ## _ENUM; enum N

#if Build_CompilerMSVC
# pragma section(".roglob", read)
# define ReadOnly __declspec(allocate(".roglob"))
#elif Build_ModeDebug
# error ReadOnly not implemented for target platform
#endif

Function void *M_Copy(void *dest, const void *src, size_t bytes);
Function void
Swap_(unsigned char *a,
      unsigned char *b,
      int64_t bytes_per_item)
{
    unsigned char temp[1024];
    int64_t w = sizeof(temp);
    if(bytes_per_item < w)
    {
        w = bytes_per_item;
    }
    
    for(int64_t i = 0;
        i < bytes_per_item;
        i += w)
    {
        M_Copy(temp, a + i, w);
        M_Copy(a + i, b + i, w);
        M_Copy(b + i, temp, w);
    }
}
#define Swap(A, B) Swap_(&(A), &(B), Min(sizeof(A), sizeof(B)))

//~NOTE(tbt): type generic math helpers
#define Min(A, B) ((A) < (B) ? (A) : (B))
#define Max(A, B) ((A) > (B) ? (A) : (B))
#define Clamp(X, A, B) (Max(A, Min(B, X)))
#define Abs(A) ((A) < 0 ? -(A) : (A))

#define MinInF(A) InF_((A), ArrayCount(A), Min1F)
#define MaxInF(A) InF_((A), ArrayCount(A), Max1F)
#define MinInI(A) InI_((A), ArrayCount(A), Min1I)
#define MaxInI(A) InI_((A), ArrayCount(A), Max1I)
#define MinInU(A) InU_((A), ArrayCount(A), Min1U)
#define MaxInU(A) InU_((A), ArrayCount(A), Max1U)

Function float
InF_(float a[], size_t count,
     float( *compare)(float a, float b))
{
    float result = a[0];
    for(size_t i = 1;
        i < count;
        i += 1)
    {
        result = compare(a[i], result);
    }
    return result;
}

Function int
InI_(int a[], size_t count,
     int( *compare)(int a, int b))
{
    int result = a[0];
    for(size_t i = 1;
        i < count;
        i += 1)
    {
        result = compare(a[i], result);
    }
    return result;
}

Function size_t
InU_(size_t a[], size_t count,
     size_t( *compare)(size_t a, size_t b))
{
    size_t result = a[0];
    for(size_t i = 1;
        i < count;
        i += 1)
    {
        result = compare(a[i], result);
    }
    return result;
}

//~NOTE(tbt): pointer arithmetic
#define IntFromPtr(P) ((uintptr_t)((unsigned char *)(P) - (unsigned char *)0))
#define PtrFromInt(N) ((void *)((unsigned char *)0 + (N)))

#define Member(T, M) (&((T *)NULL)->M)
#define OffsetOf(T, M) IntFromPtr(Member(T, M))

#define Statement(S) do { S } while(False)

//~NOTE(tbt): asserts

#if Build_EnableAsserts
# if Build_OSWindows
#  define AssertBreak_() __debugbreak()
# else
#  define AssertBreak_() volatile int i = *((int *)NULL)
# endif
#else
# define AssertBreak_() (NULL)
#endif

typedef void AssertLogHook(char *title, char *msg);
Global AssertLogHook *assert_log = NULL;

Function void
Assert_(int c, char *msg)
{
#if Build_EnableAsserts
    if(!c)
    {
        if(NULL != assert_log)
        {
            assert_log("Assertion Failure", msg);
        }
        
        AssertBreak_();
    }
#endif
}

#ifdef Build_EnableAsserts
# define Assert(C) Statement(Assert_((C), __FILE__ "(" Stringify(__LINE__) "): assertion failure: " Stringify_(C));)
#endif

//~NOTE(tbt): misc types

typedef enum
{
    False = 0,
    True = 1,
} Bool;

typedef unsigned int DataAccessFlags;
typedef enum
{
    DataAccessFlags_Read    = Bit(0),
    DataAccessFlags_Write   = Bit(1),
    DataAccessFlags_Execute = Bit(2),
} DataAccessFlags_ENUM;

//~NOTE(tbt): signed/unsigned encoding

Function uint64_t
I64EncodeAsU64(int64_t a)
{
    uint64_t result = 0;
    result |= (uint64_t)a <<  1;
    result |= (uint64_t)a >> 63;
    return result;
}

Function int64_t
I64DecodeFromU64(uint64_t a)
{
    int64_t result = a >> 1;
    if(a & 1)
    {
        result *= -1;
    }
    return result;
}
