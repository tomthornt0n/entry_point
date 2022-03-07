
Function void
LINUX_EventQueueUpdate(LINUX_EventQueue *queue)
{
    free(queue->prev);
    queue->prev = queue->current;
    queue->current = queue->next;
    if(linux_g_app.should_block_to_wait_for_events && !linux_g_app.should_force_next_update)
    {
        queue->next = xcb_wait_for_event(linux_g_app.connection);
    }
    else
    {
        queue->next = xcb_poll_for_queued_event(linux_g_app.connection);
    }
}

Function void
LINUX_WindowMake(LINUX_Window *window, S8 title, V2I dimensions, W_DrawHook draw)
{
    window->frame_arena = M_ArenaMake(m_default_hooks);
    window->events = EV_QueueMake();
    
    unsigned int value_mask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
    
    window->window = xcb_generate_id(linux_g_app.connection);
    xcb_create_window(linux_g_app.connection,
                      XCB_COPY_FROM_PARENT,
                      window->window,
                      linux_g_app.screen->root,
                      0, 0, dimensions.x, dimensions.y,
                      0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      linux_g_app.visual_id,
                      value_mask, linux_g_app.value_list);
    
    xcb_map_window(linux_g_app.connection, window->window);
    
    xcb_atom_t window_close_atom = linux_g_app.window_close_reply->atom;
    xcb_change_property(linux_g_app.connection,
                        XCB_PROP_MODE_REPLACE,
                        window->window,
                        linux_g_app.wm_protocol_reply->atom,
                        XCB_ATOM_ATOM,
                        32,
                        1,
                        &window_close_atom);
    
    xcb_change_property(linux_g_app.connection,
                        XCB_PROP_MODE_REPLACE,
                        window->window,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        title.len, title.buffer);
    
    window->drawable = glX.CreateWindow(linux_g_app.display, linux_g_app.framebuffer_config, window->window, 0);
    glX.MakeContextCurrent(linux_g_app.display, window->drawable, window->drawable, linux_g_app.context);
    
    if(0 == window->window)
    {
        xcb_destroy_window(linux_g_app.connection, window->window);
    }
    
    if(!linux_r_data.is_initialised)
    {
        // TODO(tbt): figure out if i need to do this per window or can get away with doing it once
#if Build_ModeDebug
        gl.Enable(GL_DEBUG_OUTPUT);
        gl.Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        gl.DebugMessageCallback(LINUX_GLDebugOutput, 0);
#endif
        gl.Enable(GL_BLEND);
        gl.BlendEquation(GL_FUNC_ADD);
        gl.BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        //-NOTE(tbt): nil sprite
        Pixel nil_sprite_data = { 0xffffffff };
        gl.GenTextures(1, &linux_sprite_nil.id);
        gl.BindTexture(GL_TEXTURE_2D, linux_sprite_nil.id);
        gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        gl.TexImage2D(GL_TEXTURE_2D, 0,
                      GL_RGBA8,
                      1, 1, 0,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      &nil_sprite_data);
        
        //-NOTE(tbt): vao
        gl.GenVertexArrays(1, &linux_r_data.vao);
        gl.BindVertexArray(linux_r_data.vao);
        
        //-NOTE(tbt): vbo
        gl.GenBuffers(1, &linux_r_data.vbo);
        gl.BindBuffer(GL_ARRAY_BUFFER, linux_r_data.vbo);
        gl.BufferData(GL_ARRAY_BUFFER, R_Batch_MaxVertices*sizeof(R_Vertex), 0, GL_DYNAMIC_DRAW);
        gl.EnableVertexAttribArray(0);
        gl.EnableVertexAttribArray(1);
        gl.EnableVertexAttribArray(2);
        gl.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(R_Vertex), Member(R_Vertex, position));
        gl.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(R_Vertex), Member(R_Vertex, uv));
        gl.VertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(R_Vertex), Member(R_Vertex, colour));
        
        //-NOTE(tbt): ibo
        gl.GenBuffers(1, &linux_r_data.ibo);
        gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, linux_r_data.ibo);
        gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, R_Batch_MaxIndices*sizeof(GLuint), 0, GL_DYNAMIC_DRAW);
        
        //-NOTE(tbt): default shader
        linux_r_data.default_shader = gl.CreateProgram();
        const char *vert_src =
            "#version 330 core\n"
            "layout(location=0) in vec2  a_position;\n"
            "layout(location=1) in vec2  a_texture_coordinates;\n"
            "layout(location=2) in vec4  a_colour;\n"
            "out vec4 v_colour;\n"
            "out vec2 v_texture_coordinates;\n"
            "uniform mat4 u_projection_matrix;\n"
            "void main()\n"
            "{\n"
            "    v_colour = a_colour;\n"
            "    v_texture_coordinates = a_texture_coordinates;\n"
            "    gl_Position = u_projection_matrix * vec4(a_position, 0.0, 1.0);\n"
            "}\n";
        int vert = gl.CreateShader(GL_VERTEX_SHADER);
        gl.ShaderSource(vert, 1, &vert_src, 0);
        gl.CompileShader(vert);
        gl.AttachShader(linux_r_data.default_shader, vert);
        const char *frag_src =
            "#version 330 core\n"
            "layout(location=0) out vec4 o_colour;\n"
            "in vec4 v_colour;\n"
            "in vec2 v_texture_coordinates;\n"
            "uniform sampler2D u_texture;\n"
            "void main()\n"
            "{\n"
            "    o_colour = texture2D(u_texture, v_texture_coordinates) * v_colour;\n"
            "}\n";
        int frag = gl.CreateShader(GL_FRAGMENT_SHADER);
        gl.ShaderSource(frag, 1, &frag_src, 0);
        gl.CompileShader(frag);
        gl.AttachShader(linux_r_data.default_shader, frag);
        gl.LinkProgram(linux_r_data.default_shader);
        gl.DeleteShader(vert);
        gl.DeleteShader(frag);
        
        //-NOTE(tbt): uniforms
        linux_r_data.projection_matrix_location = gl.GetUniformLocation(linux_r_data.default_shader, "u_projection_matrix");
        Assert(linux_r_data.projection_matrix_location >= 0);
        
        linux_r_data.is_initialised = True;
    }
    
    window->draw = draw;
}

Function void
LINUX_WindowDestroy(LINUX_Window *window)
{
    glX.DestroyWindow(linux_g_app.display, window->drawable);
    xcb_destroy_window(linux_g_app.connection, window->window);
    M_ArenaDestroy(&window->frame_arena);
}

Function void
LINUX_WindowDraw(LINUX_Window *window)
{
    if(0 != window->draw)
    {
        M_ArenaClear(&window->frame_arena);
        gl.Clear(GL_COLOR_BUFFER_BIT);
        R_CmdQueueRecord(&window->r_cmd_queue)
        {
            window->draw(window);
        }
        R_CmdQueueSort(&window->r_cmd_queue);
        R_CmdQueueExec(&window->r_cmd_queue, window);
    }
}

Function Bool
W_Update(W_Handle window)
{
    LINUX_Window *w = window;
    
    w->frame_start = w->frame_end;
    w->frame_end = T_SecondsGet();
    
    EV_QueueClear(&w->events);
    
    LINUX_EventQueueUpdate(&w->event_queue);
    while(0 != w->event_queue.current)
    {
        // TODO(tbt): get modifiers from event state field
        I_Modifiers modifiers = 0;
        if(w->is_key_down[I_Key_Ctrl])
        {
            modifiers |= I_Modifiers_Ctrl;
        }
        if(w->is_key_down[I_Key_Shift])
        {
            modifiers |= I_Modifiers_Shift;
        }
        if(w->is_key_down[I_Key_Alt])
        {
            modifiers |= I_Modifiers_Alt;
        }
        
        switch(w->event_queue.current->response_type & ~0x80)
        {
            default: break;
            
            case(XCB_BUTTON_PRESS):
            case(XCB_BUTTON_RELEASE):
            {
                xcb_button_press_event_t *event = (xcb_button_press_event_t *)w->event_queue.current;
                if(event->detail == 4 || event->detail == 5 ||
                   event->detail == 6 || event->detail == 7)
                {
                    V2F delta = U2F(0.0f);
                    if(4 == event->detail)
                    {
                        delta.y = +1.0f;
                    }
                    else if(5 == event->detail)
                    {
                        delta.y = -1.0f;
                    }
                    else if(6 == event->detail)
                    {
                        delta.x = -1.0f;
                    }
                    else if(7 == event->detail)
                    {
                        delta.x = +1.0f;
                    }
                    EV_Data event =
                    {
                        .kind = EV_Kind_MouseScroll,
                        .modifiers = modifiers,
                        .position = delta,
                    };
                    EV_QueuePush(&w->events, event);
                }
                else
                {
                    I_Key key = I_Key_None;
                    if(event->detail == 1)
                    {
                        key = I_Key_MouseButtonLeft;
                    }
                    else if(event->detail == 2)
                    {
                        key = I_Key_MouseButtonMiddle;
                    }
                    else if(event->detail == 3)
                    {
                        key = I_Key_MouseButtonRight;
                    }
                    else if(event->detail == 9)
                    {
                        key = I_Key_MouseButtonForward;
                    }
                    else if(event->detail == 8)
                    {
                        key = I_Key_MouseButtonBackward;
                    }
                    
                    EV_Data ev =
                    {
                        .kind = EV_Kind_Key,
                        .modifiers = modifiers,
                        .key = key,
                        .is_down = (XCB_BUTTON_PRESS == (event->response_type & ~0x80)),
                        .position = W_MousePositionGet(w),
                    };
                    EV_QueuePush(&w->events, ev);
                    w->is_key_down[ev.key] = ev.is_down;
                }
            } break;
            
            case(XCB_KEY_PRESS):
            case(XCB_KEY_RELEASE):
            {
                xcb_key_press_event_t *event = (xcb_key_press_event_t *)w->event_queue.current;
                
                I_Key key = 0;
                if(event->detail == 49)
                {
                    key = I_Key_GraveAccent;
                }
                else if(event->detail == 19)
                {
                    key = I_Key_0;
                }
                else if(event->detail == 10)
                {
                    key = I_Key_1;
                }
                else if(event->detail == 11)
                {
                    key = I_Key_2;
                }
                else if(event->detail == 12)
                {
                    key = I_Key_3;
                }
                else if(event->detail == 13)
                {
                    key = I_Key_4;
                }
                else if(event->detail == 14)
                {
                    key = I_Key_5;
                }
                else if(event->detail == 15)
                {
                    key = I_Key_6;
                }
                else if(event->detail == 16)
                {
                    key = I_Key_7;
                }
                else if(event->detail == 17)
                {
                    key = I_Key_8;
                }
                else if(event->detail == 18)
                {
                    key = I_Key_9;
                }
                else if(event->detail == 20)
                {
                    key = I_Key_Minus;
                }
                else if(event->detail == 21)
                {
                    key = I_Key_Equal;
                }
                else if(event->detail == 22)
                {
                    key = I_Key_Backspace;
                }
                else if(event->detail == 38)
                {
                    key = I_Key_A;
                }
                else if(event->detail == 56)
                {
                    key = I_Key_B;
                }
                else if(event->detail == 54)
                {
                    key = I_Key_C;
                }
                else if(event->detail == 40)
                {
                    key = I_Key_D;
                }
                else if(event->detail == 26)
                {
                    key = I_Key_E;
                }
                else if(event->detail == 41)
                {
                    key = I_Key_F;
                }
                else if(event->detail == 42)
                {
                    key = I_Key_G;
                }
                else if(event->detail == 43)
                {
                    key = I_Key_H;
                }
                else if(event->detail == 31)
                {
                    key = I_Key_I;
                }
                else if(event->detail == 44)
                {
                    key = I_Key_J;
                }
                else if(event->detail == 45)
                {
                    key = I_Key_K;
                }
                else if(event->detail == 46)
                {
                    key = I_Key_L;
                }
                else if(event->detail == 58)
                {
                    key = I_Key_M;
                }
                else if(event->detail == 57)
                {
                    key = I_Key_N;
                }
                else if(event->detail == 32)
                {
                    key = I_Key_O;
                }
                else if(event->detail == 33)
                {
                    key = I_Key_P;
                }
                else if(event->detail == 24)
                {
                    key = I_Key_Q;
                }
                else if(event->detail == 27)
                {
                    key = I_Key_R;
                }
                else if(event->detail == 39)
                {
                    key = I_Key_S;
                }
                else if(event->detail == 28)
                {
                    key = I_Key_T;
                }
                else if(event->detail == 30)
                {
                    key = I_Key_U;
                }
                else if(event->detail == 55)
                {
                    key = I_Key_V;
                }
                else if(event->detail == 25)
                {
                    key = I_Key_W;
                }
                else if(event->detail == 53)
                {
                    key = I_Key_X;
                }
                else if(event->detail == 29)
                {
                    key = I_Key_Y;
                }
                else if(event->detail == 52)
                {
                    key = I_Key_Z;
                }
                else if(event->detail == 65)
                {
                    key = I_Key_Space;
                }
                else if(event->detail == 60)
                {
                    key = I_Key_Period;
                }
                else if(event->detail == 61)
                {
                    key = I_Key_ForwardSlash;
                }
                else if(event->detail == 59)
                {
                    key = I_Key_Comma;
                }
                else if(event->detail == 48)
                {
                    key = I_Key_Quote;
                }
                else if(event->detail == 34)
                {
                    key = I_Key_LeftBracket;
                }
                else if(event->detail == 35)
                {
                    key = I_Key_RightBracket;
                }
                else if(event->detail == 23)
                {
                    key = I_Key_Tab;
                }
                else if(event->detail == 67)
                {
                    key = I_Key_F1;
                }
                else if(event->detail == 68)
                {
                    key = I_Key_F2;
                }
                else if(event->detail == 69)
                {
                    key = I_Key_F3;
                }
                else if(event->detail == 70)
                {
                    key = I_Key_F4;
                }
                else if(event->detail == 71)
                {
                    key = I_Key_F5;
                }
                else if(event->detail == 72)
                {
                    key = I_Key_F6;
                }
                else if(event->detail == 73)
                {
                    key = I_Key_F7;
                }
                else if(event->detail == 74)
                {
                    key = I_Key_F8;
                }
                else if(event->detail == 75)
                {
                    key = I_Key_F9;
                }
                else if(event->detail == 76)
                {
                    key = I_Key_F10;
                }
                else if(event->detail == 95)
                {
                    key = I_Key_F11;
                }
                else if(event->detail == 96)
                {
                    key = I_Key_F12;
                }
                else if(event->detail == 119)
                {
                    key = I_Key_Delete;
                }
                else if(event->detail == 118)
                {
                    key = I_Key_Insert;
                }
                else if(event->detail == 9)
                {
                    key = I_Key_Esc;
                }
                else if(event->detail == 36)
                {
                    key = I_Key_Enter;
                }
                else if(event->detail == 111)
                {
                    key = I_Key_Up;
                }
                else if(event->detail == 113)
                {
                    key = I_Key_Left;
                }
                else if(event->detail == 116)
                {
                    key = I_Key_Down;
                }
                else if(event->detail == 114)
                {
                    key = I_Key_Right;
                }
                else if(event->detail == 112)
                {
                    key = I_Key_PageUp;
                }
                else if(event->detail == 117)
                {
                    key = I_Key_PageDown;
                }
                else if(event->detail == 110)
                {
                    key = I_Key_Home;
                }
                else if(event->detail == 115)
                {
                    key = I_Key_End;
                }
                else if(event->detail == 47)
                {
                    key = I_Key_Colon;
                }
                else if(event->detail == 37)
                {
                    key = I_Key_Ctrl;
                }
                else if(event->detail == 50)
                {
                    key = I_Key_Shift;
                }
                else if(event->detail == 64)
                {
                    key = I_Key_Alt;
                }
                
                EV_Data ev =
                {
                    .kind = EV_Kind_Key,
                    .modifiers = modifiers,
                    .key = key,
                    .is_down = (XCB_KEY_PRESS == (event->response_type & ~0x80)),
                    .position = W_MousePositionGet(w),
                };
                EV_QueuePush(&w->events, ev);
                w->is_key_down[ev.key] = ev.is_down;
                
                if(ev.is_down)
                {
                    XKeyEvent x_key_event =
                    {
                        .display = linux_g_app.display,
                        .keycode = event->detail,
                        .state = event->state,
                    };
                    char buffer[16] = {0};
                    if(XLookupString(&x_key_event, buffer, sizeof(buffer), 0, 0))
                    {
                        size_t codepoint = CodepointFromUTF8(CStringAsS8(buffer), 0).codepoint;
                        if(codepoint >= 32 && codepoint != 127)
                        {
                            EV_Data ev = 
                            {
                                .kind = EV_Kind_Char,
                                .modifiers = modifiers,
                                .codepoint = codepoint,
                            };
                            EV_QueuePush(&w->events, ev);
                        }
                    }
                }
            } break;
            
            case(XCB_MOTION_NOTIFY):
            {
                EV_Data event =
                {
                    .kind = EV_Kind_MouseMove,
                    .modifiers = modifiers,
                    .position = W_MousePositionGet(w),
                };
                V2F delta = Sub2F(event.position, w->mouse_position);
                event.size = V2I(delta.x + 0.5f, delta.y + 0.5f);
                EV_QueuePush(&w->events, event);
                
                w->mouse_position = event.position;
            } break;
            
            case(XCB_LEAVE_NOTIFY):
            case(XCB_FOCUS_OUT):
            {
                EV_Data event =
                {
                    .kind = EV_Kind_MouseLeave,
                    .modifiers = modifiers,
                };
                EV_QueuePush(&w->events, event);
                M_Set(w->is_key_down, False, sizeof(w->is_key_down));
            } break;
            
            case(XCB_EXPOSE):
            {
                V2I window_dimensions = W_DimensionsGet(w);
                EV_Data event =
                {
                    .kind = EV_Kind_WindowSize,
                    .modifiers = modifiers,
                    .size = window_dimensions,
                };
                EV_QueuePush(&w->events, event);
                w->dimensions = window_dimensions;
                
                w->projection_matrix = OrthoMake4x4F(0.0f, window_dimensions.x, 0.0f, window_dimensions.y, 0.0f, 1.0f);
                
                LINUX_WindowDraw(w);
            } break;
            
            case(XCB_CLIENT_MESSAGE):
            {
                xcb_client_message_event_t *event = (xcb_client_message_event_t *)w->event_queue.current;
                if(event->data.data32[0] == linux_g_app.window_close_reply->atom)
                {
                    w->should_close = True;
                }
            } break;
        }
        LINUX_EventQueueUpdate(&w->event_queue);
    }
    
    glX.MakeCurrent(linux_g_app.display, w->drawable, linux_g_app.context);
    LINUX_WindowDraw(window);
    
    Bool result = !w->should_close;
    return result;
}

Function V2I
W_DimensionsGet(W_Handle window)
{
    V2I result = {0};
    
    LINUX_Window *w = window;
    
    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(linux_g_app.connection, w->window);
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(linux_g_app.connection, cookie, 0);
    if (0 != reply)
    {
        result.x = reply->width;
        result.y = reply->height;
    }
    free(reply);
    
    return result;
}

Function I2F
W_ClientRectGet(W_Handle window)
{
    V2I dimensions = W_DimensionsGet(window);
    I2F result = I2F(V2F(0.0f, 0.0f), V2F(dimensions.x, dimensions.y));
    return result;
}

Function V2F
W_MousePositionGet(W_Handle window)
{
    V2F result = {0};
    
    LINUX_Window *w = window;
    
    xcb_query_pointer_cookie_t cookie = xcb_query_pointer(linux_g_app.connection, w->window);
    xcb_query_pointer_reply_t *reply = xcb_query_pointer_reply(linux_g_app.connection, cookie, 0);
    if (0 != reply)
    {
        // TODO(tbt): relative to window, not root
        result.x = reply->win_x;
        result.y = reply->win_y;
    }
    free(reply);
    
    return result;
}

Function Bool
W_KeyStateGet(W_Handle window, I_Key key)
{
    LINUX_Window *w = window;
    Bool result = w->is_key_down[key];
    return result;
}

Function I_Modifiers
W_ModifiersMaskGet(W_Handle window)
{
    I_Modifiers result = {0};
    if(W_KeyStateGet(window, I_Key_Ctrl))
    {
        result |= I_Modifiers_Ctrl;
    }
    if(W_KeyStateGet(window, I_Key_Alt))
    {
        result |= I_Modifiers_Alt;
    }
    if(W_KeyStateGet(window, I_Key_Shift))
    {
        result |= I_Modifiers_Shift;
    }
    return result;
}

Function void
W_FullscreenSet(W_Handle window, Bool is_fullscreen)
{
    LINUX_Window *w = window;
    
    xcb_intern_atom_reply_t *wm_state_reply = LINUX_XAtomReplyGet("_NET_WM_STATE");
    xcb_intern_atom_reply_t *fullscreen_reply = LINUX_XAtomReplyGet("_NET_WM_STATE_FULLSCREEN");
    
    if (is_fullscreen)                           
    {                                         
        xcb_change_property(linux_g_app.connection,    
                            XCB_PROP_MODE_REPLACE,
                            w->window,
                            wm_state_reply->atom,
                            XCB_ATOM_ATOM,
                            32,
                            1,
                            &fullscreen_reply->atom);
    }
    else 
    {
        xcb_delete_property(linux_g_app.connection, w->window, wm_state_reply->atom);
    }   
    
    free(fullscreen_reply);
    free(wm_state_reply);
    
    xcb_unmap_window(linux_g_app.connection, w->window);  
    xcb_map_window(linux_g_app.connection, w->window);
}

Function void
W_VSyncSet(W_Handle window, Bool is_vsync)
{
    LINUX_Window *w = window;
    if(LINUX_GLHasExtension(linux_g_app.default_screen_id, "GLX_EXT_swap_control"))
    {
        glX.SwapIntervalEXT(linux_g_app.display, w->drawable, is_vsync);
    }
    else if(LINUX_GLHasExtension(linux_g_app.default_screen_id, "GLX_SGI_swap_control"))
    {
        glX.SwapIntervalSGI(is_vsync);
    }
    else if(LINUX_GLHasExtension(linux_g_app.default_screen_id, "GLX_MESA_swap_control"))
    {
        glX.SwapIntervalMESA(is_vsync);
    }
}

Function void
W_CursorKindSet(W_Handle window, W_CursorKind kind)
{
    LINUX_Window *w = window;
    
    xcb_font_t font = xcb_generate_id(linux_g_app.connection);
    xcb_open_font(linux_g_app.connection, font, strlen("cursor"), "cursor");
    
    int id = 2;
    if(kind < W_CursorKind_MAX)
    {
        W_CursorKind lut[W_CursorKind_MAX] =
        {
            [W_CursorKind_Default] = 2,
            [W_CursorKind_HResize] = 108,
            [W_CursorKind_VResize] = 116,
            [W_CursorKind_Hidden] = 0,
        };
        id = lut[kind];
    }
    
    xcb_cursor_t cursor = xcb_generate_id(linux_g_app.connection);
    xcb_create_glyph_cursor(linux_g_app.connection,
                            cursor,
                            font, font,
                            id, id + 1,
                            0, 0, 0, 0, 0, 0);
    
    int mask = XCB_CW_CURSOR;
    uint32_t value_list = cursor;
    xcb_change_window_attributes(linux_g_app.connection, w->window, mask, &value_list);
    
    xcb_free_cursor(linux_g_app.connection, cursor);
    xcb_close_font(linux_g_app.connection, font);
}


Function double
W_FrameTimeGet(W_Handle window)
{
    LINUX_Window *w = window;
    double result = w->frame_end - w->frame_start;
    return result;
}

Function M_Arena *
W_FrameArenaGet(W_Handle window)
{
    LINUX_Window *w = window;
    M_Arena *result = &w->frame_arena;
    return result;
}

Function EV_Queue *
W_EventQueueGet(W_Handle window)
{
    LINUX_Window *w = window;
    EV_Queue *result = &w->events;
    return result;
}

Function void
W_UserDataSet(W_Handle window, void *data)
{
    LINUX_Window *w = window;
    w->user_data = data;
}

Function void *
W_UserDataGet(W_Handle window)
{
    LINUX_Window *w = window;
    void *result = w->user_data;
    return result;
}
