
#include <sys/time.h>
#include <xcb/xcb.h>

typedef enum
{
    LINUX_ClipAtoms_Targets,
    LINUX_ClipAtoms_Multiple,
    LINUX_ClipAtoms_Timestamp,
    LINUX_ClipAtoms_Incr,
    LINUX_ClipAtoms_Clipboard,
    LINUX_ClipAtoms_Utf8String,
    LINUX_ClipAtoms_TextUriList,
    LINUX_ClipAtoms_MAX,
} LINUX_ClipAtoms;

Global S8 linux_clip_atoms_ids[LINUX_ClipAtoms_MAX] =
{
    S8Initialiser("TARGETS"),
    S8Initialiser("MULTIPLE"),
    S8Initialiser("TIMESTAMP"),
    S8Initialiser("INCR"),
    S8Initialiser("CLIPBOARD"),
    S8Initialiser("UTF8_STRING"),
    S8Initialiser("text/uri-list"),
};

typedef struct
{
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_atom_t atoms[LINUX_ClipAtoms_MAX];
    xcb_window_t window;
    
    Thread event_loop;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    
    Bool has_ownership;
    M_Arena arena;
    S8 data;
    xcb_atom_t target;
    xcb_atom_t mode;
} LINUX_ClipState;
Global LINUX_ClipState linux_clip_state = {0};


Function S8
LINUX_URIFromFilename(M_Arena *arena, S8 filename)
{
    // TODO(tbt): this is pretty hacky but should do the job
    
    S8 result = {0};
    
    S8 scheme = S8("file://");
    
    for(size_t i = 0; i < filename.len; i += 1)
    {
        result.len += 1;
        if(!CharIsAlphanumeric(filename.buffer[i]) &&
           filename.buffer[i] != '/' &&
           filename.buffer[i] != '$' &&
           filename.buffer[i] != '-' &&
           filename.buffer[i] != '_' &&
           filename.buffer[i] != '.' &&
           filename.buffer[i] != '+' &&
           filename.buffer[i] != '!' &&
           filename.buffer[i] != '*' &&
           filename.buffer[i] != '\'' &&
           filename.buffer[i] != '(' &&
           filename.buffer[i] != ')')
        {
            result.len += 2;
        }
    }
    result.len += scheme.len;
    
    result.buffer = M_ArenaPush(arena, result.len + 1);
    M_Copy(result.buffer, scheme.buffer, scheme.len);
    
    size_t o = scheme.len;
    for(size_t i = 0; i < result.len; i += 1)
    {
        if(!CharIsAlphanumeric(filename.buffer[i]) &&
           filename.buffer[i] != '/' &&
           filename.buffer[i] != '$' &&
           filename.buffer[i] != '-' &&
           filename.buffer[i] != '_' &&
           filename.buffer[i] != '.' &&
           filename.buffer[i] != '+' &&
           filename.buffer[i] != '!' &&
           filename.buffer[i] != '*' &&
           filename.buffer[i] != '\'' &&
           filename.buffer[i] != '(' &&
           filename.buffer[i] != ')')
        {
            result.buffer[o++] = '%';
            stbsp_sprintf(&result.buffer[o++], "%x", filename.buffer[i]);
        }
        else
        {
            result.buffer[o] = filename.buffer[i];
        }
        o += 1;
    }
    
    return result;
}

Function S8
LINUX_FilenameFromURI(M_Arena *arena, S8 uri)
{
    // TODO(tbt): not great but probably enough to get by
    
    S8 result = {0};
    
    if(S8Consume(&uri, S8("file://")))
    {
        for(size_t i = 0; i < uri.len; i += 1)
        {
            if('%' == uri.buffer[i])
            {
                i += 2;
            }
            result.len += 1;
        }
        result.buffer = M_ArenaPush(arena, result.len + 1);
        
        size_t o = 0;
        for(size_t i = 0; i < uri.len; i += 1)
        {
            if('%' == uri.buffer[i])
            {
                char c = 0;
                for(int j = 1; j <= 2; j += 1)
                {
                    unsigned char byte = uri.buffer[i + j];
                    if(byte >= '0' && byte <= '9')
                    {
                        byte = byte - '0';
                    }
                    else if(byte >= 'a' && byte <='f')
                    {
                        byte = byte - 'a' + 10;
                    }
                    else if(byte >= 'A' && byte <='F')
                    {
                        byte = byte - 'A' + 10;    
                    }
                    c = (c << 4) | (byte & 0xF);
                }
                i += 2;
                result.buffer[o] = c;
            }
            else
            {
                result.buffer[o] = uri.buffer[i];
            }
            o += 1;
        }
    }
    
    return result;
}

Function void
LINUX_ClipEventLoop(void *user_data)
{
    xcb_generic_event_t *ev;
    for(;;)
    {
        ev = xcb_wait_for_event(linux_clip_state.connection);
        
        if(0 == ev->response_type)
        {
            xcb_generic_error_t *e = (xcb_generic_error_t *)ev;
            fprintf(stderr, "received X error %d\n", e->error_code);
            free(e);
        }
        else
        {
            switch(ev->response_type & ~0x80)
            {
                default: break;
                
                case(XCB_DESTROY_NOTIFY):
                {
                    xcb_destroy_notify_event_t *destroy_notify = (xcb_destroy_notify_event_t *)ev;
                    if(linux_clip_state.window == destroy_notify->window)
                    {
                        return;
                    }
                } break;
                
                case(XCB_SELECTION_CLEAR):
                {
                    xcb_selection_clear_event_t *selection_clear = (xcb_selection_clear_event_t *)ev;
                    if(selection_clear->owner == linux_clip_state.window &&
                       selection_clear->selection == linux_clip_state.mode)
                    {
                        pthread_mutex_lock(&linux_clip_state.lock);
                        M_ArenaClear(&linux_clip_state.arena);
                        linux_clip_state.data = (S8){0};
                        linux_clip_state.has_ownership = False;
                        linux_clip_state.target = XCB_NONE;
                        pthread_mutex_unlock(&linux_clip_state.lock);
                    }
                } break;
                
                case(XCB_SELECTION_NOTIFY):
                {
                    xcb_selection_notify_event_t *selection_notify = (xcb_selection_notify_event_t *)ev;
                    if(selection_notify->property == linux_clip_state.atoms[LINUX_ClipAtoms_Clipboard])
                    {
                        M_Temp scratch = TC_ScratchGet(0, 0);
                        
                        size_t transfer_size = 1048576;
                        
                        unsigned char *buffer = 0;
                        size_t buffer_size = 0;
                        size_t bytes_after = 1;
                        uint8_t actual_format;
                        xcb_atom_t actual_type;
                        xcb_get_property_reply_t *reply = 0;
                        
                        while(bytes_after > 0)
                        {
                            free(reply);
                            xcb_get_property_cookie_t cookie = xcb_get_property(linux_clip_state.connection,
                                                                                True, linux_clip_state.window,
                                                                                selection_notify->property,
                                                                                XCB_ATOM_ANY,
                                                                                buffer_size/ 4,
                                                                                transfer_size / 4);
                            reply = xcb_get_property_reply(linux_clip_state.connection, cookie, 0);
                            
                            Bool was_error = (0 == reply || 0 != (reply->format % 8));
                            if(buffer_size > 0)
                            {
                                was_error = was_error || (reply->format != actual_format ||
                                                          reply->type != actual_type);
                            }
                            if(was_error)
                            {
                                break;
                            }
                            else
                            {
                                if(0 == buffer_size)
                                {
                                    actual_type = reply->type;
                                    actual_format = reply->format;
                                }
                                
                                int n_items = xcb_get_property_value_length(reply);
                                if(n_items > 0)
                                {
                                    if(0 != (buffer_size % 4))
                                    {
                                        break;
                                    }
                                    
                                    // TODO(tbt): INCR format
                                    
                                    size_t size_per_item = reply->format / 8;
                                    size_t size = size_per_item * n_items;
                                    
                                    unsigned char *buffer_new = M_ArenaPushAligned(scratch.arena, size, 1);
                                    if(0 == buffer)
                                    {
                                        buffer = buffer_new;
                                    }
                                    buffer_size += size;
                                    M_Copy(buffer_new, xcb_get_property_value(reply), size);
                                }
                                
                                bytes_after = reply->bytes_after;
                            }
                        }
                        free(reply);
                        if(0 != buffer && selection_notify->property == linux_clip_state.mode)
                        {
                            pthread_mutex_lock(&linux_clip_state.lock);
                            M_ArenaClear(&linux_clip_state.arena);
                            linux_clip_state.data = S8Clone(&linux_clip_state.arena, (S8){ .buffer = buffer, .len = buffer_size });
                            linux_clip_state.target = selection_notify->target;
                            pthread_cond_broadcast(&linux_clip_state.cond);
                            pthread_mutex_unlock(&linux_clip_state.lock);
                        }
                        M_TempEnd(&scratch);
                    }
                } break;
                
                case(XCB_SELECTION_REQUEST):
                {
                    xcb_selection_request_event_t *selection_request = (xcb_selection_request_event_t *)ev;
                    
                    xcb_selection_notify_event_t notify = {0};
                    notify.response_type = XCB_SELECTION_NOTIFY;
                    notify.time = XCB_CURRENT_TIME;
                    notify.requestor = selection_request->requestor;
                    notify.selection = selection_request->selection;
                    notify.target = selection_request->target;
                    notify.property = selection_request->property;
                    
                    if(XCB_NONE == selection_request->property)
                    {
                        selection_request->property = selection_request->target;
                    }
                    
                    if(selection_request->target == linux_clip_state.atoms[LINUX_ClipAtoms_Targets])
                    {
                        xcb_atom_t targets[] = 
                        {
                            linux_clip_state.atoms[LINUX_ClipAtoms_Targets],
                            linux_clip_state.atoms[LINUX_ClipAtoms_Timestamp],
                            linux_clip_state.target,
                        };
                        xcb_change_property(linux_clip_state.connection,
                                            XCB_PROP_MODE_REPLACE,
                                            selection_request->requestor,
                                            selection_request->property,
                                            XCB_ATOM_ATOM,
                                            sizeof(xcb_atom_t)*8,
                                            ArrayCount(targets),
                                            targets);
                    }
                    else if(selection_request->target == linux_clip_state.atoms[LINUX_ClipAtoms_Timestamp])
                    {
                        xcb_timestamp_t current_time = XCB_CURRENT_TIME;
                        xcb_change_property(linux_clip_state.connection,
                                            XCB_PROP_MODE_REPLACE,
                                            selection_request->requestor,
                                            selection_request->property,
                                            XCB_ATOM_INTEGER,
                                            sizeof(current_time)*8,
                                            1, &current_time);
                    }
                    else if(selection_request->target == linux_clip_state.target)
                    {
                        pthread_mutex_lock(&linux_clip_state.lock);
                        
                        if(selection_request->selection != linux_clip_state.mode ||
                           !linux_clip_state.has_ownership ||
                           0 == linux_clip_state.data.len ||
                           linux_clip_state.target != selection_request->target)
                        {
                            notify.property = XCB_NONE;
                        }
                        else
                        {
                            xcb_change_property(linux_clip_state.connection,
                                                XCB_PROP_MODE_REPLACE,
                                                selection_request->requestor,
                                                selection_request->property,
                                                selection_request->target, 8,
                                                linux_clip_state.data.len,
                                                linux_clip_state.data.buffer);
                        }
                        
                        pthread_mutex_unlock(&linux_clip_state.lock);
                    }
                    else
                    {
                        notify.property = XCB_NONE;
                    }
                    // TODO(tbt): handle MULTIPLE as ICCCM requires
                    
                    xcb_send_event(linux_clip_state.connection,
                                   False,
                                   selection_request->requestor,
                                   XCB_EVENT_MASK_PROPERTY_CHANGE,
                                   (char *)&notify);
                    xcb_flush(linux_clip_state.connection);
                } break;
            }
        }
    }
}

Function void
LINUX_ClipInit(void)
{
    int default_screen_id;
    linux_clip_state.connection = xcb_connect(0, &default_screen_id);
    xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(linux_clip_state.connection));
    for(int i = default_screen_id; 0 != screen_iterator.rem && i > 0; i -= 1)
    {
        xcb_screen_next(&screen_iterator);
    }
    linux_clip_state.screen = screen_iterator.data;
    
    xcb_intern_atom_cookie_t cookies[LINUX_ClipAtoms_MAX];
    for(LINUX_ClipAtoms i = 0; i < LINUX_ClipAtoms_MAX; i += 1)
    {
        cookies[i] = xcb_intern_atom(linux_clip_state.connection, 0,
                                     linux_clip_atoms_ids[i].len,
                                     linux_clip_atoms_ids[i].buffer);
    }
    for(LINUX_ClipAtoms i = 0; i < LINUX_ClipAtoms_MAX; i += 1)
    {
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(linux_clip_state.connection, cookies[i], 0);
        if(0 != reply)
        {
            linux_clip_state.atoms[i] = reply->atom;
            free(reply);
        }
    }
    
    linux_clip_state.mode = linux_clip_state.atoms[LINUX_ClipAtoms_Clipboard];
    
    uint32_t event_mask = XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE;
    linux_clip_state.window = xcb_generate_id(linux_clip_state.connection);
    xcb_create_window(linux_clip_state.connection,
                      XCB_COPY_FROM_PARENT,
                      linux_clip_state.window,
                      linux_clip_state.screen->root,
                      0, 0, 10, 10, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      linux_clip_state.screen->root_visual,
                      XCB_CW_EVENT_MASK, &event_mask);
    
    linux_clip_state.arena = M_ArenaMake(m_default_hooks);
    
    pthread_mutex_init(&linux_clip_state.lock, 0);
    pthread_cond_init(&linux_clip_state.cond, 0);
    linux_clip_state.event_loop = ThreadMake(LINUX_ClipEventLoop, 0);
}

Function S8
CLIP_TextGet(M_Arena *arena)
{
    S8 result = {0};
    pthread_mutex_lock(&linux_clip_state.lock);
    
    
    if(!linux_clip_state.has_ownership)
    {
        M_ArenaClear(&linux_clip_state.arena);
        linux_clip_state.data = (S8){0};
        
        xcb_get_selection_owner_cookie_t cookie = xcb_get_selection_owner(linux_clip_state.connection, linux_clip_state.mode);
        xcb_get_selection_owner_reply_t *owner = xcb_get_selection_owner_reply(linux_clip_state.connection, cookie, 0);
        if(0 != owner && 0 != owner->owner)
        {
            linux_clip_state.target = linux_clip_state.atoms[LINUX_ClipAtoms_Utf8String];
            xcb_convert_selection(linux_clip_state.connection,
                                  linux_clip_state.window,
                                  linux_clip_state.mode,
                                  linux_clip_state.target,
                                  linux_clip_state.mode,
                                  XCB_CURRENT_TIME);
            xcb_flush(linux_clip_state.connection);
            
            size_t timeout_ms = 1500;
            
            struct timeval now;
            gettimeofday(&now, 0);
            struct timespec timeout;
            timeout.tv_sec = now.tv_sec + (timeout_ms / 1000);
            timeout.tv_nsec = (now.tv_usec * 1000UL) + ((timeout_ms % 1000) * 1000000UL);
            if(timeout.tv_nsec >= 1000000000UL)
            {
                timeout.tv_sec += timeout.tv_nsec / 1000000000UL;
                timeout.tv_nsec = timeout.tv_nsec % 1000000000UL;
            }
            
            int rc = 0;
            while(0 == rc && 0 == linux_clip_state.data.len)
            {
                rc = pthread_cond_timedwait(&linux_clip_state.cond, &linux_clip_state.lock, &timeout);
            }
        }
        free(owner);
    }
    
    if(linux_clip_state.atoms[LINUX_ClipAtoms_Utf8String] == linux_clip_state.target)
    {
        result = linux_clip_state.data;
    }
    
    pthread_mutex_unlock(&linux_clip_state.lock);
    
    return S8Clone(arena, result);
}

Function void
CLIP_TextSet(S8 string)
{
    if(0 != string.buffer && 0 != string.len)
    {
        pthread_mutex_lock(&linux_clip_state.lock);
        M_ArenaClear(&linux_clip_state.arena);
        linux_clip_state.data = S8Clone(&linux_clip_state.arena, string);
        linux_clip_state.has_ownership = True;
        linux_clip_state.target = linux_clip_state.atoms[LINUX_ClipAtoms_Utf8String];
        xcb_set_selection_owner(linux_clip_state.connection, linux_clip_state.window, linux_clip_state.mode, XCB_CURRENT_TIME);
        xcb_flush(linux_clip_state.connection);
        pthread_mutex_unlock(&linux_clip_state.lock);
    }
}

Function S8List
CLIP_FilesGet(M_Arena *arena)
{
    S8List result = {0};
    
    pthread_mutex_lock(&linux_clip_state.lock);
    
    if(!linux_clip_state.has_ownership)
    {
        M_ArenaClear(&linux_clip_state.arena);
        linux_clip_state.data = (S8){0};
        
        xcb_get_selection_owner_cookie_t cookie = xcb_get_selection_owner(linux_clip_state.connection, linux_clip_state.mode);
        xcb_get_selection_owner_reply_t *owner = xcb_get_selection_owner_reply(linux_clip_state.connection, cookie, 0);
        if(0 != owner && 0 != owner->owner)
        {
            linux_clip_state.target = linux_clip_state.atoms[LINUX_ClipAtoms_TextUriList];
            xcb_convert_selection(linux_clip_state.connection,
                                  linux_clip_state.window,
                                  linux_clip_state.mode,
                                  linux_clip_state.target,
                                  linux_clip_state.mode,
                                  XCB_CURRENT_TIME);
            xcb_flush(linux_clip_state.connection);
            
            size_t timeout_ms = 1500;
            
            struct timeval now;
            gettimeofday(&now, 0);
            struct timespec timeout;
            timeout.tv_sec = now.tv_sec + (timeout_ms / 1000);
            timeout.tv_nsec = (now.tv_usec * 1000UL) + ((timeout_ms % 1000) * 1000000UL);
            if(timeout.tv_nsec >= 1000000000UL)
            {
                timeout.tv_sec += timeout.tv_nsec / 1000000000UL;
                timeout.tv_nsec = timeout.tv_nsec % 1000000000UL;
            }
            
            int rc = 0;
            while(0 == rc && 0 == linux_clip_state.data.len)
            {
                rc = pthread_cond_timedwait(&linux_clip_state.cond, &linux_clip_state.lock, &timeout);
            }
        }
        free(owner);
    }
    
    if(linux_clip_state.atoms[LINUX_ClipAtoms_TextUriList] == linux_clip_state.target)
    {
        M_Temp scratch = TC_ScratchGet(&arena, 1);
        S8Split split = S8SplitMake(linux_clip_state.data, S8("\r\n"), 0);
        while(S8SplitNext(&split))
        {
            S8 filename = LINUX_FilenameFromURI(scratch.arena, split.current);
            if(filename.len > 0)
            {
                S8ListAppend(arena, &result, filename);
            }
        }
        M_TempEnd(&scratch);
    }
    
    pthread_mutex_unlock(&linux_clip_state.lock);
    
    return result;
}

Function void
CLIP_FilesSet(S8List filenames)
{
    M_Temp scratch = TC_ScratchGet(0, 0);
    
    S8List uri_list = {0};
    for(S8ListForEach(filenames, s))
    {
        S8ListAppend(scratch.arena, &uri_list, s->string);
    }
    
    pthread_mutex_lock(&linux_clip_state.lock);
    M_ArenaClear(&linux_clip_state.arena);
    linux_clip_state.data = S8ListJoinFormatted(&linux_clip_state.arena, uri_list, S8ListJoinFormat(.suffix = S8("\n")));
    linux_clip_state.has_ownership = True;
    linux_clip_state.target = linux_clip_state.atoms[LINUX_ClipAtoms_TextUriList];
    xcb_set_selection_owner(linux_clip_state.connection, linux_clip_state.window, linux_clip_state.mode, XCB_CURRENT_TIME);
    xcb_flush(linux_clip_state.connection);
    pthread_mutex_unlock(&linux_clip_state.lock);
    
    M_TempEnd(&scratch);
}
