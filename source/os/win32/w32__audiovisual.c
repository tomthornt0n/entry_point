typedef struct
{
    OS_AV_FrameState _;
    
    HANDLE instance_handle;
    HWND window_handle;
    
    V2I prev_mouse_position;
    WINDOWPLACEMENT prev_window_placement;
    
    // NOTE(tbt): keep track of the previous values for config members to check whether or not to update
    V2I actual_mouse_position;
    OS_CursorKind actual_cursor_kind;
    Bool actual_is_fullscreen;
} W32_AV_Ctx;

static V2I
OS_WindowDimensionsGet(OS_AV_FrameState *ctx)
{
    W32_AV_Ctx *_ctx = (W32_AV_Ctx *)ctx;
    
    V2I result;
    RECT client_rect;
    GetClientRect(_ctx->window_handle, &client_rect);
    result.x = client_rect.right - client_rect.left;
    result.y = client_rect.bottom - client_rect.top;
    return result;
    
}

static V2I
OS_MousePositionGet(OS_AV_FrameState *ctx)
{
    W32_AV_Ctx *_ctx = (W32_AV_Ctx *)ctx;
    
    V2I result = {0};
    POINT mouse;
    GetCursorPos(&mouse);
    ScreenToClient(_ctx->window_handle, &mouse);
    result.x = mouse.x;
    result.y = mouse.y;
    return result;
}

static void
W32_ToggleFullscreen(W32_AV_Ctx *ctx)
{
    DWORD window_style = GetWindowLong(ctx->window_handle, GWL_STYLE);
    
    if(window_style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if(GetWindowPlacement(ctx->window_handle, &ctx->prev_window_placement) &&
           GetMonitorInfo(MonitorFromWindow(ctx->window_handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
        {
            
            SetWindowLong(ctx->window_handle, GWL_STYLE,
                          window_style & ~WS_OVERLAPPEDWINDOW);
            
            SetWindowPos(ctx->window_handle, HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right -
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom -
                         monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(ctx->window_handle, GWL_STYLE,
                      window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ctx->window_handle, &ctx->prev_window_placement);
        SetWindowPos(ctx->window_handle, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

static LRESULT
W32_WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    
    static Bool is_mouse_hover_active = False;
    
    // TODO(tbt): 
    W32_AV_Ctx *_os = (W32_AV_Ctx *)GetWindowLongPtrW(window_handle, GWLP_USERDATA);
    OS_AV_FrameState *os = &_os->_;
    
    KeyModifiers modifiers = 0;
    if(GetKeyState(VK_CONTROL) & 0x8000)
    {
        modifiers |= KeyModifiers_Ctrl;
    }
    if(GetKeyState(VK_SHIFT) & 0x8000)
    {
        modifiers |= KeyModifiers_Shift;
    }
    if(GetKeyState(VK_MENU) & 0x8000)
    {
        modifiers |= KeyModifiers_Alt;
    }
    
    if(message == WM_CLOSE ||
       message == WM_DESTROY ||
       message == WM_QUIT)
    {
        os->is_running = False;
        result = 0;
    }
    else if(message == WM_LBUTTONDOWN)
    {
        OS_Event event =
        {
            .kind = OS_EventKind_Key,
            .modifiers = modifiers,
            .key = Key_MouseButtonLeft,
            .is_down = True,
        };
        OS_EventPush(os, &event);
        os->is_key_down[Key_MouseButtonLeft] = True;
    }
    else if(message == WM_LBUTTONUP)
    {
        OS_Event event =
        {
            .kind = OS_EventKind_Key,
            .modifiers = modifiers,
            .key = Key_MouseButtonLeft,
            .is_down = False,
        };
        OS_EventPush(os, &event);
        os->is_key_down[Key_MouseButtonLeft] = False;
    }
    else if(message == WM_MBUTTONDOWN)
    {
        OS_Event event =
        {
            .kind = OS_EventKind_Key,
            .modifiers = modifiers,
            .key = Key_MouseButtonMiddle,
            .is_down = True,
        };
        OS_EventPush(os, &event);
        os->is_key_down[Key_MouseButtonRight] = True;
    }
    else if(message == WM_MBUTTONUP)
    {
        OS_Event event =
        {
            .kind = OS_EventKind_Key,
            .modifiers = modifiers,
            .key = Key_MouseButtonMiddle,
            .is_down = False,
        };
        OS_EventPush(os, &event);
        os->is_key_down[Key_MouseButtonRight] = False;
    }
    else if(message == WM_RBUTTONDOWN)
    {
        OS_Event event =
        {
            .kind = OS_EventKind_Key,
            .modifiers = modifiers,
            .key = Key_MouseButtonRight,
            .is_down = True,
        };
        OS_EventPush(os, &event);
        os->is_key_down[Key_MouseButtonRight] = True;
    }
    else if(message == WM_RBUTTONUP)
    {
        OS_Event event =
        {
            .kind = OS_EventKind_Key,
            .modifiers = modifiers,
            .key = Key_MouseButtonRight,
            .is_down = False,
        };
        OS_EventPush(os, &event);
        os->is_key_down[Key_MouseButtonRight] = False;
    }
    else if(message == WM_MOUSEMOVE)
    {
        os->mouse_position = OS_MousePositionGet(os);
        V2I delta = Sub2I(os->mouse_position, _os->prev_mouse_position);
        
        OS_Event event =
        {
            .kind = OS_EventKind_MouseMove,
            .modifiers = modifiers,
            .position = os->mouse_position,
            .delta = V2F(delta.x, delta.y),
        };
        OS_EventPush(os, &event);
        
        _os->prev_mouse_position = os->mouse_position;
        
        if(is_mouse_hover_active == False)
        {
            is_mouse_hover_active = 1;
            TRACKMOUSEEVENT track_mouse_event = {0};
            {
                track_mouse_event.cbSize = sizeof(track_mouse_event);
                track_mouse_event.dwFlags = TME_LEAVE;
                track_mouse_event.hwndTrack = window_handle;
                track_mouse_event.dwHoverTime = HOVER_DEFAULT;
            }
            TrackMouseEvent(&track_mouse_event);
        }
    }
    else if(message == WM_MOUSELEAVE)
    {
        is_mouse_hover_active = False;
        SetCursor(LoadCursorW(NULL, IDC_ARROW));
    }
    else if(message == WM_MOUSEWHEEL)
    {
        V2F delta = V2F(0.0f, HIWORD(w_param) / 120.0f);
        OS_Event event =
        {
            .kind = OS_EventKind_MouseScroll,
            .modifiers = modifiers,
            .delta = delta,
        };
        OS_EventPush(os, &event);
        os->mouse_wheel_delta = Add2F(os->mouse_wheel_delta, delta);
    }
    else if(message == WM_MOUSEHWHEEL)
    {
        V2F delta = V2F(HIWORD(w_param) / 120.0f, 0.0f);
        OS_Event event =
        {
            .kind = OS_EventKind_MouseScroll,
            .modifiers = modifiers,
            .delta = delta,
        };
        OS_EventPush(os, &event);
        os->mouse_wheel_delta = Add2F(os->mouse_wheel_delta, delta);
    }
    else if(message == WM_SETCURSOR)
    {
        V2I window_dimensions = OS_WindowDimensionsGet(os);
        if(is_mouse_hover_active &&
           os->mouse_position.x >= 1 &&
           os->mouse_position.x < window_dimensions.x &&
           os->mouse_position.y >= 1 &&
           os->mouse_position.y < window_dimensions.y)
        {
            switch(os->cursor_kind)
            {
                case(OS_CursorKind_HResize):
                {
                    SetCursor(LoadCursorW(NULL, IDC_SIZEWE));
                    break;
                }
                case(OS_CursorKind_VResize):
                {
                    SetCursor(LoadCursorW(NULL, IDC_SIZENS));
                    break;
                }
                case(OS_CursorKind_Default):
                {
                    SetCursor(LoadCursorW(NULL, IDC_ARROW));
                    break;
                }
                default: break;
            }
        }
        else
        {
            result = DefWindowProc(window_handle, message, w_param, l_param);
        }
    }
    else if(message == WM_SIZE)
    {
        OS_Event event =
        {
            .kind = OS_EventKind_WindowSize,
            .modifiers = modifiers,
            .size = OS_WindowDimensionsGet(os),
        };
        OS_EventPush(os, &event);
    }
    else if(message == WM_SYSKEYDOWN || message == WM_SYSKEYUP ||
            message == WM_KEYDOWN || message == WM_KEYUP)
    {
        size_t vkey_code = w_param;
        Bool was_down = !!(l_param & (1 << 30));
        Bool is_down = !(l_param & (1 << 31));
        
        size_t key_input = Key_NONE;
        
        if((vkey_code >= 'A' && vkey_code <= 'Z') ||
           (vkey_code >= '0' && vkey_code <= '9'))
        {
            key_input = (vkey_code >= 'A' && vkey_code <= 'Z') ? Key_A + (vkey_code - 'A') : Key_0 + (vkey_code - '0');
        }
        else
        {
            if(vkey_code == VK_ESCAPE)
            {
                key_input = Key_Esc;
            }
            else if(vkey_code >= VK_F1 && vkey_code <= VK_F12)
            {
                key_input = Key_F1 + vkey_code - VK_F1;
            }
            else if(vkey_code == VK_OEM_3)
            {
                key_input = Key_GraveAccent;
            }
            else if(vkey_code == VK_OEM_MINUS)
            {
                key_input = Key_Minus;
            }
            else if(vkey_code == VK_OEM_PLUS)
            {
                key_input = Key_Equal;
            }
            else if(vkey_code == VK_BACK)
            {
                key_input = Key_Backspace;
            }
            else if(vkey_code == VK_TAB)
            {
                key_input = Key_Tab;
            }
            else if(vkey_code == VK_SPACE)
            {
                key_input = Key_Space;
            }
            else if(vkey_code == VK_RETURN)
            {
                key_input = Key_Enter;
            }
            else if(vkey_code == VK_CONTROL)
            {
                key_input = Key_Ctrl;
                modifiers &= ~KeyModifiers_Ctrl;
            }
            else if(vkey_code == VK_SHIFT)
            {
                key_input = Key_Shift;
                modifiers &= ~KeyModifiers_Shift;
            }
            else if(vkey_code == VK_MENU)
            {
                key_input = Key_Alt;
                modifiers &= ~KeyModifiers_Alt;
            }
            else if(vkey_code == VK_UP)
            {
                key_input = Key_Up;
            }
            else if(vkey_code == VK_LEFT)
            {
                key_input = Key_Left;
            }
            else if(vkey_code == VK_DOWN)
            {
                key_input = Key_Down;
            }
            else if(vkey_code == VK_RIGHT)
            {
                key_input = Key_Right;
            }
            else if(vkey_code == VK_DELETE)
            {
                key_input = Key_Delete;
            }
            else if(vkey_code == VK_PRIOR)
            {
                key_input = Key_PageUp;
            }
            else if(vkey_code == VK_NEXT)
            {
                key_input = Key_PageDown;
            }
            else if(vkey_code == VK_HOME)
            {
                key_input = Key_Home;
            }
            else if(vkey_code == VK_END)
            {
                key_input = Key_End;
            }
            else if(vkey_code == VK_OEM_2)
            {
                key_input = Key_ForwardSlash;
            }
            else if(vkey_code == VK_OEM_PERIOD)
            {
                key_input = Key_Period;
            }
            else if(vkey_code == VK_OEM_COMMA)
            {
                key_input = Key_Comma;
            }
            else if(vkey_code == VK_OEM_7)
            {
                key_input = Key_Quote;
            }
            else if(vkey_code == VK_OEM_4)
            {
                key_input = Key_LeftBracket;
            }
            else if(vkey_code == VK_OEM_6)
            {
                key_input = Key_RightBracket;
            }
        }
        
        OS_Event event =
        {
            .kind = OS_EventKind_Key,
            .modifiers = modifiers,
            .key = key_input,
            .is_down = is_down,
        };
        OS_EventPush(os, &event);
        os->is_key_down[key_input] = is_down;
        
        result = DefWindowProc(window_handle, message, w_param, l_param);
    }
    else if(message == WM_CHAR)
    {
        size_t char_input = w_param;
        if(char_input >= 32 && char_input != VK_RETURN && char_input != VK_ESCAPE &&
           char_input != 127)
        {
            OS_Event event =
            {
                .kind = OS_EventKind_Char,
                .modifiers = modifiers,
                .codepoint = w_param,
            };
            OS_EventPush(os, &event);
        }
    }
    else
    {
        result = DefWindowProc(window_handle, message, w_param, l_param);
    }
    
    return result;
}

static Bool
OS_AV_ContextMake(OS_AV_FrameState **result,
                  S8 window_title)
{
    W32_AV_Ctx *_result = OS_M_Reserve(sizeof(*_result));
    OS_M_Commit(_result, sizeof(*_result));
    
    _result->instance_handle = GetModuleHandle(NULL);
    
    static Bool is_window_class_registered = False;
    WNDCLASSW window_class = 
    {
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = W32_WindowProc,
        .hInstance = _result->instance_handle,
        .lpszClassName = ApplicationNameWString,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
    };
    
    // NOTE(tbt): only register class once if OS_AV_ContextMake is called multiple times
    if(is_window_class_registered || RegisterClassW(&window_class))
    {
        is_window_class_registered = True;
        
        M_Temp scratch;
        OS_TC_ScratchMem(scratch, NULL, 0)
        {
            _result->window_handle = CreateWindowW(ApplicationNameWString,
                                                   S16FromS8(scratch.arena, window_title).buffer,
                                                   WS_OVERLAPPEDWINDOW,
                                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                                   OS_DefaultWindowW, OS_DefaultWindowH,
                                                   NULL, NULL, _result->instance_handle, NULL);
            if(NULL != _result->window_handle)
            {
                //
                // NOTE(tbt): you will probably want some kind of graphics context initialisation here
                //
                
                Assert(sizeof(LONG_PTR) == sizeof(void *));
                SetWindowLongPtrW(_result->window_handle, GWLP_USERDATA, (LONG_PTR)_result);
                
                ShowWindow(_result->window_handle, SW_SHOW);
                UpdateWindow(_result->window_handle);
            }
            else
            {
                OS_AV_ContextDestroy((void *)_result);
                _result = NULL;
            }
        }
    }
    else
    {
        OS_AV_ContextDestroy((void *)_result);
        _result = NULL;
    }
    
    *result = &_result->_;
    if(result)
    {
        (*result)->frame_arena = M_ArenaMake(os_m_default_callbacks);
        (*result)->events_count = 0;
        (*result)->mouse_position = OS_MousePositionGet(*result);
        (*result)->mouse_wheel_delta = V2F(0, 0);
        (*result)->cursor_kind = OS_CursorKind_Default;
        (*result)->is_fullscreen = False;
        (*result)->target_fps = 60.0f;
        (*result)->is_running = True;
        M_Set(&((*result)->is_key_down), 0, sizeof(((*result)->is_key_down)));
    }
    return (result != NULL);
};

static Bool
OS_AV_FrameBegin(OS_AV_FrameState *ctx)
{
    W32_AV_Ctx *_ctx = (W32_AV_Ctx *)ctx;
    
    M_ArenaClear(&ctx->frame_arena);
    ctx->events_count = 0;
    
    MSG message;
    if(ctx->should_block_to_wait_for_events)
    {
        WaitMessage();
    }
    while(PeekMessageW(&message, _ctx->window_handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    
    _ctx->actual_cursor_kind = ctx->cursor_kind;
    _ctx->actual_mouse_position = ctx->mouse_position;
    _ctx->actual_is_fullscreen = ctx->is_fullscreen;
    
    return ctx->is_running;
}

static void
OS_AV_FrameEnd(OS_AV_FrameState *ctx)
{
    W32_AV_Ctx *_ctx = (W32_AV_Ctx *)ctx;
    
    Bool should_hide_cursor = ctx->cursor_kind == OS_CursorKind_Hidden;
    if((_ctx->actual_cursor_kind == OS_CursorKind_Hidden) != should_hide_cursor)
    {
        if(should_hide_cursor)
        {
            while(ShowCursor(FALSE) >= 0);
        }
        else
        {
            while(ShowCursor(TRUE) < 0);
        }
    }
    
    if(_ctx->actual_is_fullscreen != ctx->is_fullscreen)
    {
        W32_ToggleFullscreen(_ctx);
    }
}

static void
OS_AV_ContextDestroy(OS_AV_FrameState *ctx)
{
    W32_AV_Ctx *_ctx = (W32_AV_Ctx *)ctx;
    
    OS_M_Decommit(_ctx, sizeof(*_ctx));
    OS_M_Release(_ctx);
}
