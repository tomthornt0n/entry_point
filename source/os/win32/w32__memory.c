
//~NOTE(tbt): base memory

static void *
OS_M_Reserve(size_t size)
{
 return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
}

static void
OS_M_Release(void *memory)
{
 VirtualFree(memory, 0, MEM_RELEASE);
}

static void
OS_M_Commit(void *memory, size_t size)
{
 VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
}

static void
OS_M_Decommit(void *memory, size_t size)
{
 VirtualFree(memory, size, MEM_DECOMMIT);
}

//~NOTE(tbt): default arenas

static M_Arena w32_m_permanent_arena;
static M_Arena w32_m_transient_arena;

static M_Arena *
OS_PermanentArena(void)
{
 return &w32_m_permanent_arena;
}

static M_Arena *
OS_TransientArena(void)
{
 return &w32_m_transient_arena;
}
