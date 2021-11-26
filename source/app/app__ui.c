
//~NOTE(tbt): rect-cut layouts

Function I2F
UI_CutLeft(I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->min.x, rect->min.y),
        .max = V2F(rect->min.x + f, rect->max.y),
    };
    rect->min.x += f;
    return result;
}

Function I2F
UI_CutRight(I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->max.x - f, rect->min.y),
        .max = V2F(rect->max.x, rect->max.y),
    };
    rect->max.x -= f;
    return result;
}

Function I2F
UI_CutTop(I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->min.x, rect->min.y),
        .max = V2F(rect->max.x, rect->min.y + f),
    };
    rect->min.y += f;
    return result;
}

Function I2F
UI_CutBottom (I2F *rect, float f)
{
    I2F result =
    {
        .min = V2F(rect->min.x, rect->max.y - f),
        .max = V2F(rect->max.x, rect->max.y),
    };
    rect->max.y -= f;
    return result;
}

Function I2F
UI_Cut(I2F *rect, UI_CutDir dir, float f)
{
    I2F result;
    switch(dir)
    {
        case(UI_CutDir_Left):
        {
            result = UI_CutLeft(rect, f);
        } break;
        
        case(UI_CutDir_Right):
        {
            result = UI_CutRight(rect, f);
        } break;
        
        case(UI_CutDir_Top):
        {
            result = UI_CutTop(rect, f);
        } break;
        
        case(UI_CutDir_Bottom):
        {
            result = UI_CutBottom(rect, f);
        } break;
    }
    return result;
}

Function I2F
UI_GetLeft(I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.min.x, rect.min.y),
        .max = V2F(rect.min.x + f, rect.max.y),
    };
    rect.min.x += f;
    return result;
}

Function I2F
UI_GetRight(I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.max.x - f, rect.min.y),
        .max = V2F(rect.max.x, rect.max.y),
    };
    rect.max.x -= f;
    return result;
}

Function I2F
UI_GetTop(I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.min.x, rect.min.y),
        .max = V2F(rect.max.x, rect.min.y + f),
    };
    rect.min.y += f;
    return result;
}

Function I2F
UI_GetBottom (I2F rect, float f)
{
    I2F result =
    {
        .min = V2F(rect.min.x, rect.max.y - f),
        .max = V2F(rect.max.x, rect.max.y),
    };
    rect.max.y -= f;
    return result;
}

Function I2F
UI_Get(I2F rect, UI_CutDir dir, float f)
{
    I2F result;
    switch(dir)
    {
        case(UI_CutDir_Left):
        {
            result = UI_GetLeft(rect, f);
        } break;
        
        case(UI_CutDir_Right):
        {
            result = UI_GetRight(rect, f);
        } break;
        
        case(UI_CutDir_Top):
        {
            result = UI_GetTop(rect, f);
        } break;
        
        case(UI_CutDir_Bottom):
        {
            result = UI_GetBottom(rect, f);
        } break;
    }
    return result;
}


//~NOTE(tbt): scrollable rows layouts

Function void
UI_ScrollableBegin(UI_Scrollable *state,
                   I2F *rect,
                   float row_height)
{
    state->target_scroll = Min1F(state->target_scroll, 0.0f);
    state->rect = rect;
    state->height_per_row = row_height;
    float max_scroll = Max1F(state->rows_count*state->height_per_row - (rect->max.y - rect->min.y), 0.0f);
    
    W_Handle window = G_CurrentWindowGet();
    
    if(IntervalHasValue2F(*rect, W_MousePositionGet(window)))
    {
        state->target_scroll += EV_QueueScrollGet(W_EventQueueGet(window)).y;
    }
    state->target_scroll = Clamp1F(state->target_scroll, -max_scroll, 0.0f);
    
    float delta = (state->target_scroll - state->scroll) / 2.0f;
    if(Abs1F(delta) > 0.01f)
    {
        state->scroll += delta;
        G_ForceNextUpdate();
    }
    
    state->rows_count = 0;
}

Function Bool
UI_ScrollableRow(UI_Scrollable *state,
                 I2F *result)
{
    float position = state->rect->min.y + state->rows_count*state->height_per_row + state->scroll;
    *result = I2F(V2F(state->rect->min.x, position),
                  V2F(state->rect->max.x, position + state->height_per_row));
    state->rows_count += 1;
    
    return SAT2F(*result, *state->rect);
}

//~NOTE(tbt): text editing

Function void
UI_EditTextDeleteRange_(S8 *s, size_t cap,
                        I1U *selection)
{
    if(selection->cursor != selection->mark)
    {
        I1U range =
        {
            .min = Min(selection->cursor, selection->mark),
            .max = Max(selection->cursor, selection->mark),
        };
        
        M_Copy(&s->buffer[range.min],
               &s->buffer[range.max],
               cap - range.max);
        s->len -= range.max - range.min;
        
        selection->cursor = range.min;
        selection->mark = range.min;
    }
}

Function Bool
UI_EditText(char buffer[], size_t cap,
            I1U *selection,
            size_t *len)
{
    cap -= 1;
    
    S8 s = CStringAsS8(buffer);
    
    char character_insertion_buffer[5] = {0};
    
    enum
    {
        ActionFlags_MoveCursor = Bit(0),
        ActionFlags_PreDelete  = Bit(1),
        ActionFlags_PostDelete = Bit(2),
        ActionFlags_Insert     = Bit(3),
        ActionFlags_Copy       = Bit(4),
        ActionFlags_SelectAll  = Bit(5),
        
        ActionFlags_StickMark  = Bit(6),
        ActionFlags_WordLevel  = Bit(7),
        
        ActionFlags_EDIT = (ActionFlags_PreDelete |
                            ActionFlags_PostDelete |
                            ActionFlags_Insert),
    };
    struct
    {
        unsigned char flags;
        int offset;
        S8 to_insert;
    } action = {0};
    
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    EV_Queue *events = W_EventQueueGet(G_CurrentWindowGet());
    
    for(EV_QueueForEach(events, e))
    {
        M_Set(&action, 0, sizeof(action));
        
        Bool is_event_handled = False;
        
        if(EV_Kind_Key == e->kind && e->is_down)
        {
            if(I_Key_Left == e->key || I_Key_Right == e->key)
            {
                if(selection->cursor == selection->mark || (e->modifiers & I_Modifiers_Shift))
                {
                    action.flags |= ActionFlags_MoveCursor;
                    action.offset = (I_Key_Left == e->key) ? -1 : +1;
                }
                else if(I_Key_Left == e->key)
                {
                    selection->cursor = MinInU(selection->elements);
                    selection->mark = selection->cursor;
                }
                else
                {
                    selection->cursor = MaxInU(selection->elements);
                    selection->mark = selection->cursor;
                }
            }
            else if(I_Key_Home == e->key || I_Key_End == e->key)
            {
                action.flags |= ActionFlags_MoveCursor;
                if(I_Key_Home == e->key)
                {
                    action.offset = INT_MIN;
                }
                else
                {
                    action.offset = INT_MAX;
                }
            }
            else if(I_Key_Backspace == e->key || I_Key_Delete == e->key)
            {
                action.flags |= ActionFlags_PostDelete;
                if(selection->mark == selection->cursor)
                {
                    action.flags |= ActionFlags_MoveCursor;
                    action.flags |= ActionFlags_StickMark;
                    if(I_Key_Delete == e->key)
                    {
                        action.offset = +1;
                    }
                    else
                    {
                        action.offset = -1;
                    }
                }
            }
            
            if(0 != (e->modifiers & I_Modifiers_Ctrl))
            {
                if(I_Key_C == e->key || I_Key_Insert == e->key)
                {
                    action.flags |= ActionFlags_Copy;
                }
                else if(I_Key_V == e->key)
                {
                    action.flags |= ActionFlags_Insert;
                    action.flags |= ActionFlags_MoveCursor;
                    action.flags &= ~ActionFlags_StickMark;
                    action.to_insert = CLIP_TextGet(scratch.arena);
                    action.offset = S8CharIndexFromByteIndex(action.to_insert, action.to_insert.len);
                }
                else if(I_Key_X == e->key)
                {
                    action.flags |= ActionFlags_Copy;
                    action.flags |= ActionFlags_PostDelete;
                }
                else if(I_Key_A == e->key)
                {
                    action.flags = ActionFlags_SelectAll;
                }
                else
                {
                    action.flags |= ActionFlags_WordLevel;
                }
            }
            if(0 != (e->modifiers & I_Modifiers_Shift))
            {
                if(I_Key_Insert == e->key)
                {
                    action.flags |= ActionFlags_Insert;
                    action.flags |= ActionFlags_MoveCursor;
                    action.flags &= ~ActionFlags_StickMark;
                    action.to_insert = CLIP_TextGet(scratch.arena);
                    action.offset = S8CharIndexFromByteIndex(action.to_insert, action.to_insert.len);
                }
                else if(I_Key_Delete == e->key)
                {
                    action.flags |= ActionFlags_Copy;
                    action.flags |= ActionFlags_PostDelete;
                }
                else
                {
                    action.flags |= ActionFlags_StickMark;
                }
            }
            
            if(e->key < I_Key_PRINTABLE_END)
            {
                G_ForceNextUpdate();
                EV_Consume(e);
            }
        }
        else if(EV_Kind_Char == e->kind && CharIsPrintable(e->codepoint))
        {
            M_Arena str = M_ArenaFromArray(character_insertion_buffer);
            action.flags |= ActionFlags_Insert;
            action.flags |= ActionFlags_MoveCursor;
            action.flags |= ActionFlags_PreDelete;
            action.to_insert = UTF8FromCodepoint(&str, e->codepoint);
            action.offset = 1;
            G_ForceNextUpdate();
            EV_Consume(e);
        }
        
        if(action.flags & ActionFlags_SelectAll)
        {
            selection->mark = 0;
            selection->cursor = s.len;
        }
        
        if(action.flags & ActionFlags_Copy)
        {
            I1U range =
            {
                .min = Min1U(selection->cursor, selection->mark),
                .max = Max1U(selection->cursor, selection->mark),
            };
            S8 selected_string =
            {
                .buffer = &buffer[range.min],
                .len = range.max - range.min
            };
            CLIP_TextSet(selected_string);
        }
        
        if(action.flags & ActionFlags_PreDelete)
        {
            UI_EditTextDeleteRange_(&s, cap, selection);
        }
        
        if(action.flags & ActionFlags_Insert)
        {
            M_Copy(&buffer[selection->cursor + action.to_insert.len],
                   &buffer[selection->cursor],
                   cap - selection->cursor - action.to_insert.len);
            M_Copy(&buffer[selection->cursor],
                   action.to_insert.buffer,
                   action.to_insert.len);
            s.len += action.to_insert.len;
        }
        
        if(action.flags & ActionFlags_MoveCursor)
        {
            int dir = Normalise1I(action.offset);
            while(action.offset)
            {
                for(;;)
                {
                    selection->cursor += dir;
                    if(selection->cursor > s.len)
                    {
                        selection->cursor -= dir;
                        break;
                    }
                    else if(!(UTF8IsContinuationByte(s, selection->cursor) ||
                              (action.flags & ActionFlags_WordLevel) &&
                              !S8IsWordBoundary(s, selection->cursor)))
                    {
                        break;
                    }
                }
                action.offset -= dir;
                
                if(selection->cursor == 0 ||
                   selection->cursor >= s.len)
                {
                    break;
                }
            }
            selection->cursor = Clamp1U(selection->cursor, 0, s.len);
            if(!(action.flags & ActionFlags_StickMark))
            {
                selection->mark = selection->cursor;
            }
        }
        
        if(action.flags & ActionFlags_PostDelete)
        {
            UI_EditTextDeleteRange_(&s, cap, selection);
        }
    }
    
    s.buffer[s.len] = '\0';
    if(NULL != len)
    {
        *len = s.len;
    }
    
    M_TempEnd(&scratch);
    Bool is_edited = !!(action.flags & ActionFlags_EDIT);
    return is_edited;
}

Function void
UI_DrawS8WithCaret(R_Font *font,
                   S8 string,
                   V2F position,
                   V4F colour,
                   I1U *selection)
{
    R_CmdDrawS8(font, string, position, colour);
    
    if(string.len > 0)
    {
        M_Temp scratch = TC_ScratchGet(NULL, 0);
        W_Handle window = G_CurrentWindowGet();
        EV_Queue *events = W_EventQueueGet(window);
        
        R_MeasuredText mt = R_MeasureS8(scratch.arena, font, string, position);
        
        if(IntervalHasValue2F(Expand2F(mt.bounds, 4.0f), W_MousePositionGet(window)))
        {
            for(EV_QueueForEach(events, e))
            {
                if(EV_Kind_Key == e->kind &&
                   I_Key_MouseButtonLeft == e->key && e->is_down)
                {
                    R_MeasuredTextIndex clicked_char_index = R_MeasuredTextNearestIndexFromPosition(mt, W_MousePositionGet(window));
                    if(R_MeasuredTextIndex_None != clicked_char_index)
                    {
                        size_t clicked_byte_index = S8ByteIndexFromCharIndex(string, clicked_char_index);
                        selection->cursor = clicked_byte_index;
                        selection->mark = clicked_byte_index;
                        EV_Consume(e);
                    }
                }
                else if(EV_Kind_MouseMove == e->kind && W_KeyStateGet(window, I_Key_MouseButtonLeft))
                {
                    R_MeasuredTextIndex clicked_char_index = R_MeasuredTextNearestIndexFromPosition(mt, W_MousePositionGet(window));
                    if(R_MeasuredTextIndex_None != clicked_char_index)
                    {
                        size_t clicked_byte_index = S8ByteIndexFromCharIndex(string, clicked_char_index);
                        selection->cursor = clicked_byte_index + 1;
                        EV_Consume(e);
                    }
                }
            }
        }
        
        I1U char_indices =
        {
            .cursor = S8CharIndexFromByteIndex(string, selection->cursor),
            .mark = S8CharIndexFromByteIndex(string, selection->mark),
        };
        
        V2F cursor_origin = position;
        V2F mark_origin = position;
        if(char_indices.cursor > 0)
        {
            cursor_origin = V2F(mt.rects[char_indices.cursor - 1].bounds.max.x,
                                mt.rects[char_indices.cursor - 1].base_line);
        }
        if(char_indices.mark > 0)
        {
            mark_origin = V2F(mt.rects[char_indices.mark - 1].bounds.max.x,
                              mt.rects[char_indices.mark - 1].base_line);
        }
        
        if(selection->mark != selection->cursor)
        {
            I2F highlight =
            {
                .min =
                {
                    .x = Min1F(cursor_origin.x, mark_origin.x),
                    .y = cursor_origin.y - font->v_advance,
                },
                .max =
                {
                    .x = Max1F(cursor_origin.x, mark_origin.x),
                    .y = cursor_origin.y,
                },
            };
            R_CmdDrawRectFill(highlight, Scale4F(colour, 0.5f));
        }
        
        I2F caret = RectMake2F(cursor_origin, V2F(2.0f, -font->v_advance));
        R_CmdDrawRectFill(caret, colour);
        
        M_TempEnd(&scratch);
    }
    else
    {
        I2F caret = RectMake2F(position, V2F(2.0f, -font->v_advance));
        R_CmdDrawRectFill(caret, colour);
    }
}
