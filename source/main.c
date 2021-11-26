
//~NOTE(tbt): base layer
#include "base/base.h"
#include "base/base.c"

//~NOTE(tbt): os layer
#include "os/os.h"
#include "os/os.c"

//~NOTE(tbt): graphics layer
#include "graphics/graphics.h"
#include "graphics/graphics.c"

//~NOTE(tbt): app layer
#include "app/app.h"
#include "app/app.c"

//~NOTE(tbt): entry point

#if Build_NoCRT
# define EntryPoint Function int APP_EntryPoint(void)
#else
# define EntryPoint int main(int argc, char **argv)
#endif

R_Font *my_cool_font;

enum { APP_TextEditCap = 512, };

typedef struct
{
    char text_edit_backing[APP_TextEditCap];
    S8 text_edit;
    I1U text_edit_selection;
    V4F bg;
    V4F fg;
} APP_WindowData;

Function void
APP_WindowHookMake(W_Handle window)
{
    M_Arena *arena = G_ArenaFromWindow(window);
    APP_WindowData *data = M_ArenaPush(arena, sizeof(*data));
    data->text_edit.buffer = data->text_edit_backing;
    data->fg = Col(1.0f / RandIntNext(0, 10), 1.0f / RandIntNext(0, 10), 1.0f / RandIntNext(0, 10), 1.0f);
    data->bg = Col(1.0f / RandIntNext(0, 10), 1.0f / RandIntNext(0, 10), 1.0f / RandIntNext(0, 10), 1.0f);
    W_UserDataSet(window, data);
}

Function void
APP_WindowHookDraw(W_Handle window)
{
    
    APP_WindowData *data = W_UserDataGet(window);
    if(NULL != data)
    {
        R_CmdClear(data->bg);
        UI_EditText(data->text_edit_backing, APP_TextEditCap, &data->text_edit_selection, &data->text_edit.len);
        UI_DrawS8WithCaret(my_cool_font, data->text_edit, V2F(200.0f, 200.0f), data->fg, &data->text_edit_selection);
    }
}

Function void
APP_WindowHookDestroy(W_Handle window)
{
    return;
}

Function void
APP_MasterWindowHookDestroy(W_Handle window)
{
    for(G_WindowsForEach(w))
    {
        G_WindowClose(w);
    }
}

EntryPoint
{
    G_Init();
    *G_ShouldBlockToWaitForEvents() = True;
    my_cool_font = R_FontMake(S8("font.ttf"), 48);
    G_AppHooks hooks =
    {
        APP_WindowHookMake,
        APP_WindowHookDraw,
        APP_WindowHookDestroy,
    };
    W_Handle w1 = G_WindowOpen(S8("window 1"), V2I(640, 480), hooks);
    W_Handle w2 = G_WindowOpen(S8("window 2"), V2I(640, 480), hooks);
    W_Handle w3 = G_WindowOpen(S8("master window"), V2I(640, 480), G_AppHooks(APP_WindowHookMake, APP_WindowHookDraw, APP_MasterWindowHookDestroy));
    G_MainLoop();
    G_Cleanup();
    
    return 0;
}
