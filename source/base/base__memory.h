
//~NOTE(tbt): utilities

static void *M_Set(void *memory, unsigned char value, size_t bytes);
static void *M_Copy(void *dest, const void *src, size_t bytes);
static Bool M_Compare(const void *a, const void *b, size_t bytes);
enum { M_DefaultAlignment = 2 * sizeof(void *) };
static uintptr_t M_AlignForward(uintptr_t ptr, size_t align);

//~NOTE(tbt): base memory callbacks

typedef struct
{
 void *(*reserve_func)(size_t size);
 void (*release_func)(void *memory);
 void (*commit_func)(void *memory, size_t size);
 void (*decommit_func)(void *memory, size_t size);
} M_Callbacks;


//~NOTE(tbt): arenas

#if !defined(M_Arena_DefaultCap)
# define M_Arena_DefaultCap     Gigabytes(4)
#endif
#define M_Arena_CommitChunkSize Kilobytes(4)

typedef unsigned int M_ArenaFlags;
typedef enum
{
 M_ArenaFlags_Transient       = Bit(0), // NOTE(tbt): the arena acts like a ring buffer, overwriting from start when full
 M_ArenaFlags_DynamicDecommit = Bit(1), // NOTE(tbt): decommit freed memory
} M_ArenaFlags_ENUM;

typedef struct
{
 M_ArenaFlags flags;
 M_Callbacks callbacks;
 uintptr_t max;
 void *base;
 uintptr_t alloc_position;
 uintptr_t commit_position;
} M_Arena;

static M_Arena M_ArenaMake(M_Callbacks callbacks);
static M_Arena M_LocalArenaMake(void *backing, size_t size);
static M_Arena M_FixedArenaMake(M_Callbacks callbacks, size_t size);
static M_Arena M_TransientArenaMake(M_Callbacks callbacks, size_t size);
static void *M_ArenaPushAligned(M_Arena *arena, size_t size, size_t align);
static void *M_ArenaPush(M_Arena *arena, size_t size);
static void M_ArenaPop(M_Arena *arena, size_t size);
static void M_ArenaClear(M_Arena *arena);
static void M_ArenaRelease(M_Arena *arena);

//~NOTE(tbt): temporary memory

typedef struct
{
 M_Arena *arena;
 uintptr_t checkpoint_alloc_position;
} M_Temp;

static M_Temp M_TempBegin(M_Arena *arena);
static void M_TempEnd(M_Temp *temp);

//~NOTE(tbt): scratch pool

#if !defined(TC_ScratchPool_Cap)
# define TC_ScratchPool_Cap 4
#endif

typedef struct
{
 M_Arena arenas[TC_ScratchPool_Cap];
} M_ScratchPool;

static void M_ScratchPoolMake(M_ScratchPool *pool, M_Callbacks mem_callbacks);
static M_Temp M_ScratchGet(M_ScratchPool *pool, void *non_conflict[], int non_conflict_count);

