static void
OS_Init(void)
{
    //-NOTE(tbt): initialise time stuff
    LARGE_INTEGER perf_counter_frequency;
    if(QueryPerformanceFrequency(&perf_counter_frequency))
    {
        w32_t_perf_counter_ticks_per_second = perf_counter_frequency.QuadPart;
    }
    if(TIMERR_NOERROR == timeBeginPeriod(1))
    {
        w32_t_is_granular = True;
    }
    
    //-NOTE(tbt): initialise default arenas
    w32_m_permanent_arena = M_ArenaMake(os_m_default_callbacks);
    w32_m_transient_arena = M_TransientArenaMake(os_m_default_callbacks, Megabytes(128));
    
    //-NOTE(tbt): initialise thread context
    w32_tc_id = TlsAlloc();
    static OS_TC main_thread_context;
    OS_TC_Make(&main_thread_context, 0);
    OS_TC_Set(&main_thread_context);
    
    //-NOTE(tbt): initialise console
#if Build_ModeDebug
    W32_ConsoleInit();
#endif
    
    //-NOTE(tbt): initialise RNG
    RandIntInit(OS_EntropyGet());
}

static void
OS_Cleanup(void)
{
    if(w32_t_is_granular)
    {
        timeEndPeriod(1);
    }
}
