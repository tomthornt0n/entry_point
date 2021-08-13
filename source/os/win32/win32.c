
//~NOTE(tbt): platform specific system headers
#include <windows.h>

#if Build_NoCRT
# include "w32__crt_replacement.h"
int A_EntryPoint(void);
int __stdcall WinMainCRTStartup()
{
    int rc = A_EntryPoint();
    ExitProcess(rc);
}
# endif

//~NOTE(tbt): OS layer backend implementation files

#include "w32__internal.c"
#include "w32__thread_context.c"
#include "w32__memory.c"
#include "w32__console.c"
#include "w32__time.c"
#include "w32__file_io.c"
#include "w32__entropy.c"
#include "w32__shared_libraries.c"
#include "w32__init.c"
#include "w32__audiovisual.c"