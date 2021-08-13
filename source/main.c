//~NOTE(tbt): base layer
#include "base/base.h"
#include "base/base.c"

//~NOTE(tbt): os layer
#include "os/os.h"
#include "os/os.c"

//~NOTE(tbt): app

static void
PrintFileProperties(OS_F_Properties file_properties)
{
    OS_ConsoleOutputFmt(" - exists\t: %d\n"
                        " - is directory\t: %d\n"
                        " - is hidden\t: %d\n"
                        " - size\t\t: %zd bytes\n",
                        !!(file_properties.flags & OS_F_PropertiesFlags_Exists),
                        !!(file_properties.flags & OS_F_PropertiesFlags_IsDirectory),
                        !!(file_properties.flags & OS_F_PropertiesFlags_Hidden),
                        file_properties.size);
}

static void
PrintDateTime(T_DateTime date_time)
{
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S8 human_readable_time = T_S8FromDateTime(scratch.arena, date_time);
        OS_ConsoleOutputFmt("%.*s\n", Unravel(human_readable_time));
    }
}

static void
PrintDenseTime(T_DenseTime dense_time)
{
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S8 human_readable_time = T_S8FromDenseTime(scratch.arena, dense_time);
        OS_ConsoleOutputFmt("%.*s\n", Unravel(human_readable_time));
    }
}

int
A_EntryPoint(void)
{
    OS_Init();
    
    OS_AV_FrameState *gctx;
    if(OS_AV_ContextMake(&gctx, S8("test window")))
    {
        while(OS_AV_FrameBegin(gctx))
        {
            // NOTE(tbt): do stuff here
#if 1
            for(OS_Event *e = gctx->events;
                (e - gctx->events) < gctx->events_count;
                e += 1)
            {
                if(e->kind == OS_EventKind_Char)
                {
                    M_Temp scratch;
                    OS_TC_ScratchMem(scratch, NULL, 0)
                    {
                        S8 string = UTF8FromCodepoint(scratch.arena, e->codepoint);
                        OS_ConsoleOutputFmt("%.*s\n", Unravel(string));
                    }
                }
            }
#endif
            
            OS_AV_FrameEnd(gctx);
        }
        OS_AV_ContextDestroy(gctx);
    }
    
    OS_Cleanup();
    
    return 0;
}
