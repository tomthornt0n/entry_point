
//~NOTE(tbt): includes

//-NOTE(tbt): base layer
#include "base/base.h"
#include "base/base.c"

//-NOTE(tbt): os layer
#include "os/os.h"
#include "os/os.c"

//-NOTE(tbt): graphics layer
#include "graphics/graphics.h"
#include "graphics/graphics.c"

//-NOTE(tbt): collections
#include "collections/collections.c"

//~NOTE(tbt): entry point

#if Build_NoCRT
# define EntryPoint Function int APP_EntryPoint(void)
#else
# define EntryPoint int main(int argc, char **argv)
#endif

Function void
Init(W_Handle window)
{
}

Function void
UpdateAndRender(W_Handle window)
{
    R_CmdClear(Col(1.0f, 1.0f, 1.0f, 1.0f));
}

Function void
Cleanup(W_Handle window)
{
}

Global G_AppHooks example_app_hooks =
{
    Init,
    UpdateAndRender,
    Cleanup,
};

EntryPoint
{
    G_Init();
    G_WindowOpen(S8("hello world"), V2I(1024, 768), example_app_hooks);
    G_MainLoop();
    G_Cleanup();
    return 0;
}

