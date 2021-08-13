
//~NOTE(tbt): preprocessor helpers
#define Glue_(a, b) a ## b
#define Glue Glue_(a, b)

#define Stringify_(a) #a
#define Stringify(a) Stringify_(a)
#define WidenString(s) Glue_(L, s)
#define WideStringify(a) WidenString(Stringify(a))

//~NOTE(tbt): size helpers
#define Bytes(n)      Glue_(n, LLU)
#define Kilobytes(n)  (1000LLU * (uint64_t)Bytes(n))
#define Megabytes(n)  (1000LLU * (uint64_t)Kilobytes(n))
#define Gigabytes(n)  (1000LLU * (uint64_t)Megabytes(n))
#define Terabytes(n)  (1000LLU * (uint64_t)Gigabytes(n))

//~NOTE(tbt): useful constants
#define Pi (3.1415926535897f)

//~NOTE(tbt): misc helpers
#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define Bit(n) (1 << n)

#define ApplicationNameString Stringify(ApplicationName)
#define ApplicationNameWString WideStringify(ApplicationName)

#define DeferLoop(begin, end) for(Bool _i = ((begin), True); _i; ((end), _i = False))

#define StructEq(a, b) (sizeof(a) == sizeof(b) && M_Compare(&(a), &(b), sizeof(a)))

static int
WrapToBounds(int a, int min, int max)
{
    Assert(max > min);
    int result = Abs1I(a);
    result %= max - min;
    result += min;
    return result;
}

//~NOTE(tbt): type generic math helpers
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Clamp(x, a, b) (Max(a, Min(b, x)))

//~NOTE(tbt): pointer arithmetic
#define IntFromPtr(p) ((uintptr_t))((unsigned char *)(p) - (unsigned char *)0)
#define PtrFromInt(n) (void *)((unsigned char *)0 + (n))

#define Member(t, m) (((t *)0)->m)
#define OffsetOf(t, m) IntFromPtr(Member(t, m))

#define Statement(s) do { s } while(False)

//~NOTE(tbt): asserts
#if OS_Windows
# define AssertBreak_() __debugbreak()
#else
# define AssertBreak_() int i = *((int *)NULL)
#endif

#if Build_EnableAsserts
# define Assert(c) Statement( if(!(c)) { AssertBreak_(); } )
#else
# define Assert(c) 
#endif

//~NOTE(tbt): misc types
typedef enum
{
    False = 0,
    True = 1,
} Bool;

typedef enum
{
    DataAccessFlags_Read = Bit(0),
    DataAccessFlags_Write = Bit(1),
    DataAccessFlags_Execute = Bit(2),
} DataAccessFlags;

//~NOTE(tbt): signed/unsigned encoding

static uint64_t
I64EncodeAsU64(int64_t a)
{
    uint64_t result = 0;
    result |= (uint64_t)a <<  1;
    result |= (uint64_t)a >> 63;
    return result;
}

static int64_t
I64DecodeFromU64(uint64_t a)
{
    int64_t result = a >> 1;
    if(a & 1)
    {
        result *= -1;
    }
    return result;
}
