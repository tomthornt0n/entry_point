
//~NOTE(tbt): base thread context type and functions

typedef struct
{
 M_ScratchPool scratch_pool;
 int logical_thread_index;
} OS_ThreadContext;
typedef OS_ThreadContext OS_TC;

static void OS_TC_Make_(OS_TC *tctx);                          // NOTE(tbt): platform specific thread context struct init
static void OS_TC_Make(OS_TC *tctx, int logical_thread_index); // NOTE(tbt): thread context struct init
static OS_TC *OS_TC_Get(void);                                 // NOTE(tbt): gets the thread local context pointer
static void OS_TC_Set(OS_TC *ptr);                             // NOTE(tbt): sets the thread local context pointer

//~NOTE(tbt): wrappers and helpers

static M_Temp OS_TC_ScratchMemGet(void **non_conflict, int non_conflict_count);
#define OS_TC_ScratchMem(var, non_conflict, non_conflict_count) DeferLoop((var) = OS_TC_ScratchMemGet(non_conflict, non_conflict_count), M_TempEnd(&(var)))