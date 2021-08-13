
//~NOTE(tbt): base thread context type and functions

static void
OS_TC_Make(OS_TC *tctx, int logical_thread_index)
{
 M_ScratchPoolMake(&tctx->scratch_pool, os_m_default_callbacks);
 tctx->logical_thread_index = logical_thread_index;
 
 // NOTE(tbt): os specific thread context init
 OS_TC_Make_(tctx);
}

//~NOTE(tbt): wrappers and helpers

static M_Temp
OS_TC_ScratchMemGet(void *non_conflict[], int non_conflict_count)
{
 M_Temp result;
 M_ScratchPool *pool = &OS_TC_Get()->scratch_pool;
 result = M_ScratchGet(pool, non_conflict, non_conflict_count);
 return result;
}