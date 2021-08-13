
//~NOTE(tbt): platform agnostic utility implementation files
#include "os__thread_context.c"
#include "os__console.c"
#include "os__time.c"
#include "os__audiovisual.c"

//~NOTE(tbt): include relevant platform specific backend
#if OS_Windows
# include "win32/win32.c"
#else
# error no backend for current platform
#endif
