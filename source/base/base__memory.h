
//~NOTE(tbt): utilities

Function void *M_Set     (void *dest, unsigned char value, size_t bytes);
Function void *M_Copy    (void *dest, const void *src, size_t bytes);
Function Bool  M_Compare (const void *a, const void *b, size_t bytes);

enum { M_DefaultAlignment = 2 * sizeof(void *) };
Function uintptr_t M_AlignForward (uintptr_t ptr, size_t align);

//~NOTE(tbt): base memory callbacks

typedef struct
{
    void *(*reserve_func)  (size_t size);
    void  (*release_func)  (void *memory);
    void  (*commit_func)   (void *memory, size_t size);
    void  (*decommit_func) (void *memory, size_t size);
} M_Hooks;


//~NOTE(tbt): arenas

ReadOnly Global size_t m_arena_default_cap = Gigabytes(4);
ReadOnly Global size_t m_arena_commit_chunk_size = Kilobytes(4);

typedef struct
{
    M_Hooks callbacks;
    uintptr_t max;
    void *base;
    uintptr_t alloc_position;
    uintptr_t commit_position;
} M_Arena;

Function M_Arena M_ArenaMake          (M_Hooks callbacks);              // NOTE(tbt): default arena make - arena is backed by a large reserved area of virtual memory, which is commited as needed
Function M_Arena M_ArenaMakeSized     (M_Hooks callbacks, size_t size); // NOTE(tbt): default arena make - arena is backed by a large reserved area of virtual memory, which is commited as needed
Function M_Arena M_ArenaMakeFixed     (M_Hooks callbacks, size_t size); // NOTE(tbt): makes an arena of a fixed size - memory is commited at arena creation time, rather than as allocations take place
Function M_Arena M_ArenaMakeLocal     (void *backing, size_t size);     // NOTE(tbt): makes an arena of a fixed size backed by user provided memory
Function void    M_ArenaDestroy       (M_Arena *arena);                 // NOTE(tbt): destroys the arena, releasing backing memory
#define M_ArenaFromArray(B) M_ArenaMakeLocal((B), sizeof(B))

Function void   *M_ArenaPush          (M_Arena *arena, size_t size);               // NOTE(tbt): pushed `size` bytes onto the arena, using the default alignment
Function void   *M_ArenaPushAligned   (M_Arena *arena, size_t size, size_t align); // NOTE(tbt): pushes `size` bytes onto the arena, alligned to a boundary of `align`
Function void    M_ArenaPop           (M_Arena *arena, size_t size);               // NOTE(tbt): pops `size` bytes off the arena
Function void    M_ArenaClear         (M_Arena *arena);                            // NOTE(tbt): resets all of the arena's allocations

//~NOTE(tbt): temporary memory

typedef struct
{
    M_Arena *arena;
    uintptr_t checkpoint_alloc_position;
} M_Temp;

Function M_Temp M_TempBegin (M_Arena *arena);
Function void   M_TempEnd   (M_Temp *temp);

//~NOTE(tbt): scratch pool

enum { M_ScratchPool_Cap = 8, };
typedef struct
{
    M_Arena arenas[M_ScratchPool_Cap];
} M_ScratchPool;

Function void   M_ScratchPoolMake    (M_ScratchPool *pool, M_Hooks mem_callbacks);
Function M_Temp M_ScratchGet         (M_ScratchPool *pool, M_Arena *non_conflict[], int non_conflict_count);
Function void   M_ScratchPoolDestroy (M_ScratchPool *pool);

