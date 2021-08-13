
//~NOTE(tbt): base memory

static void *OS_M_Reserve(size_t size);
static void OS_M_Release(void *memory);
static void OS_M_Commit(void *memory, size_t size);
static void OS_M_Decommit(void *memory, size_t size);

static const M_Callbacks os_m_default_callbacks =
{
 .reserve_func = OS_M_Reserve,
 .release_func = OS_M_Release,
 .commit_func = OS_M_Commit,
 .decommit_func = OS_M_Decommit,
};

//~NOTE(tbt): default arenas

static M_Arena *OS_PermanentArena(void);
static M_Arena *OS_TransientArena(void);
