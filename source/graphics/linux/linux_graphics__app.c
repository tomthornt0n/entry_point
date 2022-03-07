
Function S8
LINUX_GLS8FromGLError(GLenum error)
{
    switch(error)
    {
#define LINUX_S8FromGLErrorDef(S) case(S): return S8(#S)
        LINUX_S8FromGLErrorDef(GL_NO_ERROR);
        LINUX_S8FromGLErrorDef(GL_INVALID_ENUM);
        LINUX_S8FromGLErrorDef(GL_INVALID_VALUE);
        LINUX_S8FromGLErrorDef(GL_INVALID_OPERATION);
        LINUX_S8FromGLErrorDef(GL_INVALID_FRAMEBUFFER_OPERATION);
        LINUX_S8FromGLErrorDef(GL_OUT_OF_MEMORY);
        LINUX_S8FromGLErrorDef(GL_STACK_UNDERFLOW);
        LINUX_S8FromGLErrorDef(GL_STACK_OVERFLOW);
#undef LINUX_S8FromGLErrorDef
    }
    return S8("not an error");
}

Function xcb_intern_atom_reply_t *
LINUX_XAtomReplyGet(char *atom_id)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(linux_g_app.connection,
                                                      0,
                                                      strlen(atom_id),
                                                      atom_id);
    xcb_flush(linux_g_app.connection);
    
    return xcb_intern_atom_reply(linux_g_app.connection, cookie, 0);
}

Function Bool
LINUX_GLHasExtension(int screen, char *extension)
{
    Bool result = False;
    
    const char *start = glX.QueryExtensionsString(linux_g_app.display, screen);
    if(0 != start)
    {
        for (;;)
        {
            char *at = strstr(start, extension);
            if (0 != at)
            {
                char *terminator = at + strlen(extension);
                if (at == start || *(at - 1) == ' ')
                {
                    if (*terminator == ' ' || *terminator == '\0')
                    {
                        result = True;
                        break;
                    }
                }
                start = terminator;
            }
        }
    }
    
    return result;
}

Function LINUX_GLProc *
LINUX_GLProcLoad(char *function)
{
    LINUX_GLProc *result = glXGetProcAddressARB((const GLubyte *)function);
    return result;
}

Function void
LINUX_GLLoadAll(void)
{
#define LINUX_GlProcDef(TYPE, FUNCTION) gl.FUNCTION = (PFNGL ## TYPE ## PROC)LINUX_GLProcLoad(Stringify(gl ## FUNCTION));
#include "linux_graphics__gl_proc_list.h"
    
    glX.ChooseFBConfig          = (PFNGLXCHOOSEFBCONFIGPROC         )LINUX_GLProcLoad("glXChooseFBConfig");
    glX.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)LINUX_GLProcLoad("glXCreateContextAttribsARB");
    glX.CreateNewContext        = (PFNGLXCREATENEWCONTEXTPROC       )LINUX_GLProcLoad("glXCreateNewContext");
    glX.CreateWindow            = (PFNGLXCREATEWINDOWPROC           )LINUX_GLProcLoad("glXCreateWindow");
    glX.DestroyContext          = (PFNGLXDESTROYCONTEXTPROC         )LINUX_GLProcLoad("glXDestroyContext");
    glX.DestroyWindow           = (PFNGLXDESTROYWINDOWPROC          )LINUX_GLProcLoad("glXDestroyWindow");
    glX.GetFBConfigAttrib       = (PFNGLXGETFBCONFIGATTRIBPROC      )LINUX_GLProcLoad("glXGetFBConfigAttrib");
    glX.MakeCurrent             = (PFNGLXMAKECURRENTPROC            )LINUX_GLProcLoad("glXMakeCurrent");
    glX.MakeContextCurrent      = (PFNGLXMAKECONTEXTCURRENTPROC     )LINUX_GLProcLoad("glXMakeContextCurrent");
    glX.QueryExtensionsString   = (PFNGLXQUERYEXTENSIONSSTRINGPROC  )LINUX_GLProcLoad("glXQueryExtensionsString");
    glX.SwapBuffers             = (PFNGLXSWAPBUFFERSPROC            )LINUX_GLProcLoad("glXSwapBuffers");
    glX.SwapIntervalEXT         = (PFNGLXSWAPINTERVALEXTPROC        )LINUX_GLProcLoad("glXSwapIntervalEXT");
    glX.SwapIntervalMESA        = (PFNGLXSWAPINTERVALMESAPROC       )LINUX_GLProcLoad("glXSwapIntervalMESA");
    glX.SwapIntervalSGI         = (PFNGLXSWAPINTERVALSGIPROC        )LINUX_GLProcLoad("glXSwapIntervalSGI");
}

#if Build_ModeDebug
void APIENTRY LINUX_GLDebugOutput(GLenum source,
                                  GLenum type,
                                  unsigned int id,
                                  GLenum severity,
                                  GLsizei length,
                                  const char *message,
                                  const void *user_data)
{
    M_Arena arena = M_ArenaMakeSized(m_default_hooks, Megabytes(4));
    
    S8List msg_list = {0};
    switch(source)
    {
        default:                               S8ListAppend(&arena, &msg_list, S8("UNKNOWN SOURCE\n"));
        case(GL_DEBUG_SOURCE_API):             S8ListAppend(&arena, &msg_list, S8("Source: API\n")); break;
        case(GL_DEBUG_SOURCE_WINDOW_SYSTEM):   S8ListAppend(&arena, &msg_list, S8("Source: Window system\n")); break;
        case(GL_DEBUG_SOURCE_SHADER_COMPILER): S8ListAppend(&arena, &msg_list, S8("Source: Shader compiler\n")); break;
        case(GL_DEBUG_SOURCE_THIRD_PARTY):     S8ListAppend(&arena, &msg_list, S8("Source: Third party\n")); break;
        case(GL_DEBUG_SOURCE_APPLICATION):     S8ListAppend(&arena, &msg_list, S8("Source: Application\n")); break;
        case(GL_DEBUG_SOURCE_OTHER):           S8ListAppend(&arena, &msg_list, S8("Source: Other\n")); break;
    }
    switch(type)
    {
        default:                                  S8ListAppend(&arena, &msg_list, S8("UNKNOWN TYPE\n"));
        case(GL_DEBUG_TYPE_ERROR):                S8ListAppend(&arena, &msg_list, S8("Type: Error\n")); break;
        case(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR):  S8ListAppend(&arena, &msg_list, S8("Type: Deprecated behaviour\n")); break;
        case(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR):   S8ListAppend(&arena, &msg_list, S8("Type: Undefined behaviour\n")); break;
        case(GL_DEBUG_TYPE_PORTABILITY):          S8ListAppend(&arena, &msg_list, S8("Type: Portability\n")); break;
        case(GL_DEBUG_TYPE_PERFORMANCE):          S8ListAppend(&arena, &msg_list, S8("Type: Performance\n")); break;
        case(GL_DEBUG_TYPE_MARKER):               S8ListAppend(&arena, &msg_list, S8("Type: Marker\n")); break;
        case(GL_DEBUG_TYPE_PUSH_GROUP):           S8ListAppend(&arena, &msg_list, S8("Type: Push group\n")); break;
        case(GL_DEBUG_TYPE_POP_GROUP):            S8ListAppend(&arena, &msg_list, S8("Type: Pop group\n")); break;
        case(GL_DEBUG_TYPE_OTHER):                S8ListAppend(&arena, &msg_list, S8("Type: Other\n")); break;
    }
    switch(severity)
    {
        default:                              S8ListAppend(&arena, &msg_list, S8("UNKNOWN SEVERITY\n"));
        case(GL_DEBUG_SEVERITY_HIGH):         S8ListAppend(&arena, &msg_list, S8("Severity: High\n")); break;
        case(GL_DEBUG_SEVERITY_MEDIUM):       S8ListAppend(&arena, &msg_list, S8("Severity: Medium\n")); break;
        case(GL_DEBUG_SEVERITY_LOW):          S8ListAppend(&arena, &msg_list, S8("Severity: Low\n")); break;
        case(GL_DEBUG_SEVERITY_NOTIFICATION): S8ListAppend(&arena, &msg_list, S8("Severity: Notification\n")); break;
    }
    S8ListAppend(&arena, &msg_list, (S8){ .buffer = (char *)message, .len = length});
    S8ListAppend(&arena, &msg_list, S8("\n"));
    ConsoleOutputS8(S8ListJoin(&arena, msg_list));
    M_ArenaDestroy(&arena);
    Assert(severity <= GL_DEBUG_SEVERITY_NOTIFICATION);
}
#endif

Function void
G_Init(void)
{
    OS_Init();
    
    LINUX_GLLoadAll();
    
    linux_g_app.display = XOpenDisplay(0);
    if(0 == linux_g_app.display)
    {
        ConsoleOutputS8(S8("ERROR - could not open X display"));
    }
    else
    {
        linux_g_app.default_screen_id = DefaultScreen(linux_g_app.display);
        
        linux_g_app.connection = XGetXCBConnection(linux_g_app.display);
        
        if(0 == linux_g_app.connection)
        {
            ConsoleOutputS8(S8("ERROR - could not get XCB connection from Display\n"));
        }
        else
        {
            XSetEventQueueOwner(linux_g_app.display, XCBOwnsEventQueue);
            
            xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(linux_g_app.connection));
            for(int i = linux_g_app.default_screen_id; 0 != screen_iterator.rem && i > 0; i -= 1)
            {
                xcb_screen_next(&screen_iterator);
            }
            linux_g_app.screen = screen_iterator.data;
            
            int visual_attributes[] = 
            {
                GLX_X_RENDERABLE, True,
                GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                GLX_RENDER_TYPE, GLX_RGBA_BIT,
                GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
                GLX_RED_SIZE, 8,
                GLX_GREEN_SIZE, 8,
                GLX_BLUE_SIZE, 8,
                GLX_ALPHA_SIZE, 8,
                GLX_DEPTH_SIZE, 24,
                GLX_STENCIL_SIZE, 8,
                GLX_DOUBLEBUFFER, True,
                None
            };
            int framebuffer_configs_count = 0;
            GLXFBConfig *framebuffer_configs = glX.ChooseFBConfig(linux_g_app.display, linux_g_app.default_screen_id, visual_attributes, &framebuffer_configs_count);
            if(0 == framebuffer_configs_count && 0 == framebuffer_configs)
            {
                ConsoleOutputS8(S8("ERROR - error configuring framebuffer\n"));
            }
            else
            {
                linux_g_app.framebuffer_config = framebuffer_configs[0];
                
                glX.GetFBConfigAttrib(linux_g_app.display, framebuffer_configs[0], GLX_VISUAL_ID, &linux_g_app.visual_id);
                
                int context_attribs[] =
                {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
#if Build_ModeDebug
                    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#else
                    GLX_CONTEXT_FLAGS_ARB, 0,
#endif
                    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                    None
                };
                linux_g_app.context = glX.CreateContextAttribsARB(linux_g_app.display, framebuffer_configs[0], 0, True, context_attribs);
                
                if(0 == linux_g_app.context)
                {
                    ConsoleOutputS8(S8("ERROR - could not create OpenGL context\n"));
                }
                else
                {
                    xcb_colormap_t colour_map = xcb_generate_id(linux_g_app.connection);
                    xcb_create_colormap(linux_g_app.connection, XCB_COLORMAP_ALLOC_NONE, colour_map, linux_g_app.screen->root, linux_g_app.visual_id);
                    
                    
                    uint32_t event_mask = (XCB_EVENT_MASK_EXPOSURE |
                                           XCB_EVENT_MASK_KEY_PRESS |
                                           XCB_EVENT_MASK_KEY_RELEASE |
                                           XCB_EVENT_MASK_BUTTON_PRESS |
                                           XCB_EVENT_MASK_BUTTON_RELEASE |
                                           XCB_EVENT_MASK_POINTER_MOTION|
                                           XCB_EVENT_MASK_ENTER_WINDOW |
                                           XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                                           XCB_EVENT_MASK_FOCUS_CHANGE);
                    
                    linux_g_app.value_list[0] = event_mask;
                    linux_g_app.value_list[1] = colour_map;
                    linux_g_app.value_list[2] = 0;
                    
                    linux_g_app.wm_protocol_reply = LINUX_XAtomReplyGet("WM_PROTOCOLS");
                    linux_g_app.window_close_reply = LINUX_XAtomReplyGet("WM_DELETE_WINDOW");
                    
                    linux_sprite_allocator.lock = SemaphoreMake(1);
                    
                }
                free(framebuffer_configs);
            }
        }
    }
}

Function W_Handle
G_WindowOpen(S8 title,
             V2I dimensions,
             G_AppHooks hooks)
{
    //-NOTE(tbt): allocate and make window
    M_Arena arena = M_ArenaMake(m_default_hooks);
    LINUX_WindowNode *result = M_ArenaPush(&arena, sizeof(*result));
    result->arena = arena;
    result->cleanup = hooks.close;
    LINUX_WindowMake(&result->w, title, dimensions, hooks.draw);
    hooks.open(result);
    
    //-NOTE(tbt): append to app windows list
    if(0 == linux_g_app.windows_last)
    {
        Assert(0 == linux_g_app.windows_first);
        linux_g_app.windows_first = result;
        linux_g_app.windows_last = result;
    }
    else
    {
        result->prev = linux_g_app.windows_last;
        linux_g_app.windows_last->next = result;
        linux_g_app.windows_last = result;
    }
    
    return result;
}

Function void
G_WindowClose(W_Handle window)
{
    LINUX_WindowNode *w = window;
    w->w.should_close = True;
}

Function void
LINUX_WindowNodeRemove(LINUX_WindowNode *w)
{
    if(0 == w->prev)
    {
        linux_g_app.windows_first = w->next;
    }
    else
    {
        w->prev->next = w->next;
    }
    if(0 == w->next)
    {
        linux_g_app.windows_last = w->prev;
    }
    else
    {
        w->next->prev = w->prev;
    }
    
    LINUX_WindowDestroy(&w->w);
    M_ArenaDestroy(&w->arena);
}

Function W_Handle
G_WindowsFirstGet(void)
{
    W_Handle result = linux_g_app.windows_first;
    return result;
}

Function W_Handle
G_WindowsNextGet(W_Handle current)
{
    LINUX_WindowNode *result = current;
    result = result->next;
    return result;
}

Function void
G_MainLoop(void)
{
    while(0 != linux_g_app.windows_first)
    {
        linux_g_app.should_force_next_update = False;
        
        LINUX_WindowNode *next = 0;
        for(LINUX_WindowNode *w = linux_g_app.windows_first;
            0 != w;
            w = next)
        {
            next = w->next;
            linux_g_app.current_window = w;
            if(!W_Update(w))
            {
                w->cleanup(w);
                LINUX_WindowNodeRemove(w);
            }
        }
        
        for(LINUX_WindowNode *w = linux_g_app.windows_first; 0 != w; w = w->next)
        {
            glX.SwapBuffers(linux_g_app.display, w->w.drawable);
        }
    }
}

Function void
G_Cleanup(void)
{
    LINUX_WindowNode *next = 0;
    for(LINUX_WindowNode *w = linux_g_app.windows_first;
        0 != w;
        w = next)
    {
        w->cleanup(w);
        LINUX_WindowNodeRemove(w);
    }
    glX.DestroyContext(linux_g_app.display, linux_g_app.context);
    XCloseDisplay(linux_g_app.display);
    OS_Cleanup();
}

Function Bool
G_ShouldBlockToWaitForEventsGet(void)
{
    Bool result = linux_g_app.should_block_to_wait_for_events;
    return result;
}

Function void
G_ShouldBlockToWaitForEventsSet(Bool should_block_to_wait_for_events)
{
    linux_g_app.should_block_to_wait_for_events = should_block_to_wait_for_events;
}

Function void
G_ForceNextUpdate(void)
{
    linux_g_app.should_force_next_update = True;
}

Function W_Handle
G_CurrentWindowGet(void)
{
    W_Handle result = linux_g_app.current_window;
    return result;
}

Function M_Arena *
G_ArenaFromWindow(W_Handle window)
{
    LINUX_WindowNode *w = window;
    M_Arena *result = &w->arena;
    return result;
}
