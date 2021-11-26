
//~NOTE(tbt): platform agnostic utility implementation files

#include "os__thread_context.c"
#include "os__console.c"
#include "os__time.c"

//~NOTE(tbt): include relevant platform specific backend

#if Build_OSWindows
# include "win32/w32_os.c"
#else
# error no backend for current platform
#endif
