Function uint64_t
W32_FileSizeGet(HANDLE file_handle)
{
    DWORD hi_size, lo_size;
    lo_size = GetFileSize(file_handle, &hi_size);
    uint64_t result = W32_U64FromHiAndLoWords(hi_size, lo_size);
    return result;
}

Function S8
F_ReadEntire(M_Arena *arena, S8 filename)
{
    S8 result = {0};
    
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    HANDLE file_handle = CreateFileW(filename_s16.buffer,
                                     GENERIC_READ,
                                     0, 0,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     0);
    if(INVALID_HANDLE_VALUE != file_handle)
    {
        size_t n_total_bytes_to_read = W32_FileSizeGet(file_handle);
        
        M_Temp checkpoint = M_TempBegin(arena);
        result.buffer = M_ArenaPush(arena, n_total_bytes_to_read + 1);
        result.len = n_total_bytes_to_read;
        
        unsigned char *cursor = result.buffer;
        unsigned char *end = result.buffer + n_total_bytes_to_read;
        
        Bool success = True;
        while(cursor < end && success)
        {
            uint64_t _n_bytes_to_read = end - cursor;
            DWORD n_bytes_to_read = Min(UINT_MAX, _n_bytes_to_read);
            
            DWORD n_bytes_read;
            if(ReadFile(file_handle, cursor, n_bytes_to_read, &n_bytes_read, 0))
            {
                cursor += n_bytes_read;
            }
            else
            {
                success = False;
            }
        }
        
        if(!success)
        {
            result.buffer = NULL;
            result.len = 0;
            M_TempEnd(&checkpoint);
        }
        
        CloseHandle(file_handle);
    }
    M_TempEnd(&scratch);
    return result;
}

Function S8
F_ReadTextEntire(M_Arena *arena, S8 filename)
{
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    S8 raw = F_ReadEntire(scratch.arena, filename);
    S8 result = S8LFFromCRLF(arena, raw);
    M_TempEnd(&scratch);
    
    return result;
}

Function Bool
F_WriteEntire(S8 filename, S8 data)
{
    Bool success = True;
    
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    HANDLE file_handle = CreateFileW(filename_s16.buffer,
                                     GENERIC_WRITE,
                                     0, 0,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     0);
    if(INVALID_HANDLE_VALUE != file_handle)
    {
        unsigned char *cursor = data.buffer;
        unsigned char *end = data.buffer + data.len;
        
        while(cursor < end && success)
        {
            uint64_t _n_bytes_to_write = end - cursor;
            DWORD n_bytes_to_write = Min(UINT_MAX, _n_bytes_to_write);
            
            DWORD n_bytes_wrote;
            if(WriteFile(file_handle, cursor, n_bytes_to_write, &n_bytes_wrote, 0))
            {
                cursor += n_bytes_wrote;
            }
            else
            {
                success = False;
            }
        }
        CloseHandle(file_handle);
    }
    M_TempEnd(&scratch);
    return success;
}

Function DataAccessFlags
W32_FilePropertiesFlagsFromFileAttribs(DWORD attribs)
{
    F_PropertiesFlags result = F_PropertiesFlags_Exists;
    if(attribs & FILE_ATTRIBUTE_DIRECTORY)
    {
        result |= F_PropertiesFlags_IsDirectory;
    }
    if(attribs & FILE_ATTRIBUTE_HIDDEN)
    {
        result |= F_PropertiesFlags_Hidden;
    }
    return result;
}

Function DataAccessFlags
W32_DataAccessFlagsFromFileAttribs(DWORD attribs)
{
    DataAccessFlags result = DataAccessFlags_Read | DataAccessFlags_Execute;
    if(!(attribs & FILE_ATTRIBUTE_READONLY))
    {
        result |= DataAccessFlags_Write;
    }
    return result;
}

Function F_Properties
F_PropertiesGet(S8 filename)
{
    F_Properties result = {0};
    
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    
    WIN32_FILE_ATTRIBUTE_DATA attribs = {0};
    if(GetFileAttributesExW(filename_s16.buffer, GetFileExInfoStandard, &attribs))
    {
        result.flags = W32_FilePropertiesFlagsFromFileAttribs(attribs.dwFileAttributes);
        result.size = W32_U64FromHiAndLoWords(attribs.nFileSizeHigh, attribs.nFileSizeLow);
        result.access_flags = W32_DataAccessFlagsFromFileAttribs(attribs.dwFileAttributes);
        result.creation_time = W32_T_DenseTimeFromFileTime(attribs.ftCreationTime);
        result.access_time = W32_T_DenseTimeFromFileTime(attribs.ftLastAccessTime);
        result.write_time = W32_T_DenseTimeFromFileTime(attribs.ftLastWriteTime);
    }
    M_TempEnd(&scratch);
    return result;
}

Function Bool
F_Delete(S8 filename)
{
    Bool success;
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    success = DeleteFileW(filename_s16.buffer);
    M_TempEnd(&scratch);
    return success;
}

Function Bool
F_Move(S8 filename, S8 new_filename)
{
    Bool success;
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    S16 new_filename_s16 = S16FromS8(scratch.arena, new_filename);
    success = MoveFileExW(filename_s16.buffer, new_filename_s16.buffer, MOVEFILE_COPY_ALLOWED);
    M_TempEnd(&scratch);
    return success;
}

Function Bool
F_DirectoryMake(S8 filename)
{
    Bool success;
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    success = CreateDirectoryW(filename_s16.buffer, 0);
    M_TempEnd(&scratch);
    return success;
}

Function Bool
F_DirectoryDelete(S8 filename)
{
    Bool success;
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    success = RemoveDirectoryW(filename_s16.buffer);
    M_TempEnd(&scratch);
    return success;
}

typedef union
{
    F_Iterator iter;
    struct
    {
        F_Iterator _;
        HANDLE handle;
        WIN32_FIND_DATAW find_data;
        Bool is_done;
    };
} W32_FileIterator;

Function F_Iterator *
F_IteratorMake(M_Arena *arena, S8 directory)
{
    W32_FileIterator *result = M_ArenaPush(arena, sizeof(*result));
    result->_.directory = S8Clone(arena, directory);
    
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    S16 directory_s16 = S16FromS8(scratch.arena, S8FromFmt(scratch.arena, "%.*s\\*", Unravel(directory)));
    result->handle = FindFirstFileW(directory_s16.buffer, &result->find_data);
    M_TempEnd(&scratch);
    
    return (F_Iterator *)result;
}

Function Bool
F_IteratorNext(M_Arena *arena, F_Iterator *iter)
{
    Bool result = False;
    
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    
    W32_FileIterator *_iter = (W32_FileIterator *)iter;
    if(_iter->handle != NULL &&
       _iter->handle != INVALID_HANDLE_VALUE)
    {
        while(!_iter->is_done)
        {
            wchar_t *file_name = _iter->find_data.cFileName;
            //NOTE(lucas): Windows fails to get icons for reparse points at a 50% hit rate so right now lets ignore them...
            //maybe we could keep them and give them our own icon?
            Bool reparse_point = (_iter->find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
            // NOTE(tbt): ommit the directories '.' and '..'
            Bool should_ommit = (file_name[0] == '.' && (file_name[1] == '\0' || (file_name[1] == '.' && file_name[2] == '\0'))) || reparse_point;
            
            if(!should_ommit)
            {
                iter->current_name = S8FromS16(arena, CStringAsS16(file_name));
                iter->current_full_path = AbsolutePathFromRelativePath(arena, FilenamePush(scratch.arena, iter->directory, iter->current_name));
                iter->current_properties.size = W32_U64FromHiAndLoWords(_iter->find_data.nFileSizeHigh, _iter->find_data.nFileSizeLow);
                iter->current_properties.flags = W32_FilePropertiesFlagsFromFileAttribs(_iter->find_data.dwFileAttributes);
                iter->current_properties.access_flags = W32_DataAccessFlagsFromFileAttribs(_iter->find_data.dwFileAttributes);
                iter->current_properties.creation_time = W32_T_DenseTimeFromFileTime(_iter->find_data.ftCreationTime);
                iter->current_properties.access_time = W32_T_DenseTimeFromFileTime(_iter->find_data.ftLastAccessTime);
                iter->current_properties.write_time = W32_T_DenseTimeFromFileTime(_iter->find_data.ftLastWriteTime);
                result = True;
            }
            
            if(!FindNextFileW(_iter->handle, &_iter->find_data))
            {
                _iter->is_done = True;
            }
            
            if(!should_ommit)
            {
                break;
            }
        }
    }
    
    M_TempEnd(&scratch);
    
    return result;
}

Function void
F_IteratorDestroy(F_Iterator *iter)
{
    W32_FileIterator *_iter = (W32_FileIterator *)iter;
    if(_iter->handle != NULL &&
       _iter->handle != INVALID_HANDLE_VALUE)
    {
        FindClose(_iter->handle);
        _iter->handle = NULL;
    }
}

static S8
F_StdPathGet(M_Arena *arena, F_StdPath path)
{
    S8 result = {0};
    
    M_Temp scratch = TC_ScratchGet(&arena, 1);
    switch(path)
    {
        case(F_StdPath_CWD):
        {
            size_t cap = 2048;
            wchar_t *buffer = M_ArenaPush(scratch.arena, cap * sizeof(*buffer));
            DWORD size = GetCurrentDirectoryW(cap, buffer);
            if(size > cap)
            {
                M_TempEnd(&scratch);
                buffer = M_ArenaPush(scratch.arena, size + 1);
                GetCurrentDirectoryW(size + 1, buffer);
            }
            result = S8FromS16(arena, CStringAsS16(buffer));
        } break;
        
        case(F_StdPath_ExecutableFile):
        case(F_StdPath_ExecutableDir):
        {
            if(NULL == TC_Get()->exe_path.buffer)
            {
                size_t cap = 2048;
                for(size_t r = 0; r < 4; r += 1, cap *= 4)
                {
                    wchar_t *buffer = M_ArenaPush(scratch.arena, cap * sizeof(*buffer));
                    DWORD size = GetModuleFileNameW(NULL, buffer, cap);
                    if(size == cap && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        M_TempEnd(&scratch);
                    }
                    else
                    {
                        TC_Get()->exe_path = S8FromS16(&TC_Get()->permanent_arena, CStringAsS16(buffer));
                    }
                }
                
                // NOTE(tbt): will fail if the length of the path to the exe > cap * 4 ^ 4
            }
            
            if(F_StdPath_ExecutableFile == path)
            {
                result = S8Clone(arena, TC_Get()->exe_path);
            }
            else
            {
                S8 exe_path = TC_Get()->exe_path;
                char *last_slash = exe_path.buffer;
                for(size_t char_index = 0;
                    char_index < exe_path.len;
                    char_index += 1)
                {
                    if(exe_path.buffer[char_index] == '\\')
                    {
                        last_slash = &exe_path.buffer[char_index];
                    }
                }
                result = S8Truncate(arena, exe_path, last_slash - exe_path.buffer);
            }
        } break;
        
        case(F_StdPath_Home):
        {
            HANDLE token_handle = GetCurrentProcessToken();
            DWORD size = 0;
            GetUserProfileDirectoryW(token_handle, NULL, &size);
            wchar_t *buffer = M_ArenaPush(scratch.arena, size * sizeof(*buffer));
            if(GetUserProfileDirectoryW(token_handle, buffer, &size))
            {
                result = S8FromS16(arena, CStringAsS16(buffer));
            }
        } break;
        
        case(F_StdPath_Config):
        {
            wchar_t *buffer;
            SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &buffer);
            result = S8FromS16(arena, CStringAsS16(buffer));
            CoTaskMemFree(buffer);
        } break;
        
        case(F_StdPath_Temp):
        {
            DWORD size = GetTempPathW(0, NULL);
            wchar_t *buffer = M_ArenaPush(scratch.arena, size * sizeof(*buffer));
            GetTempPathW(size, buffer);
            result = S8FromS16(arena, CStringAsS16(buffer));
            result.len -= 1; // NOTE(tbt): remove trailing '\\'
        } break;
    }
    M_TempEnd(&scratch);
    
    return result;
}

Function void
F_Exec(S8 filename, F_ExecVerb verb)
{
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    
    const wchar_t *verb_table[F_ExecVerb_MAX] =
    {
        [F_ExecVerb_Default] = NULL,
        [F_ExecVerb_Open] = L"open",
        [F_ExecVerb_Edit] = L"edit",
        [F_ExecVerb_Print] = L"print",
    };
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    ShellExecuteW(NULL, verb_table[verb], filename_s16.buffer, NULL, NULL, SW_NORMAL);
    
    M_TempEnd(&scratch);
}

Function void
CwdSet(S8 cwd)
{
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    S16 cwd_s16 = S16FromS8(scratch.arena, cwd);
    SetCurrentDirectoryW(cwd_s16.buffer);
    M_TempEnd(&scratch);
}

Function S8
AbsolutePathFromRelativePath(M_Arena *arena, S8 path)
{
    S8 result;
    
    Bool is_drive_id = ((2 == path.len) && CharIsLetter(path.buffer[0]) && (':' == path.buffer[1]));
    if(is_drive_id)
    {
        result = S8FromFmt(arena, "%.*s\\", Unravel(path));
    }
    else
    {
        M_Temp scratch = TC_ScratchGet(&arena, 1);
        S16 rel_s16 = S16FromS8(scratch.arena, path);
        size_t buf_size = GetFullPathNameW(rel_s16.buffer, 0, NULL, NULL)*sizeof(wchar_t);
        wchar_t *abs_s16 = M_ArenaPush(scratch.arena, buf_size);
        GetFullPathNameW(rel_s16.buffer, buf_size, abs_s16, NULL);
        result = S8FromS16(arena, CStringAsS16(abs_s16));
        M_TempEnd(&scratch);
    }
    
    return result;
}

Function F_ChangeHandle
F_ChangeHandleMake(S8 filename, Bool recursive)
{
    void *result = NULL;
    
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    
    S16 filename_s16 = S16FromS8(scratch.arena, AbsolutePathFromRelativePath(scratch.arena, filename));
    HANDLE file_watch_handle = FindFirstChangeNotificationW(filename_s16.buffer,
                                                            !!recursive,
                                                            FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_FILE_NAME);
    Assert(INVALID_HANDLE_VALUE != file_watch_handle);
    result = file_watch_handle;
    
    M_TempEnd(&scratch);
    return result;
}

Function Bool
F_ChangeHandleWait(F_ChangeHandle handle, size_t timeout)
{
    Bool result = False;
    if(NULL != handle)
    {
        if(F_ChangeHandleWait_Infinite == timeout)
        {
            timeout = INFINITE;
        }
        result = (WAIT_OBJECT_0 == WaitForSingleObject(handle, timeout));
        if(result)
        {
            FindNextChangeNotification(handle);
        }
    }
    return result;
}

Function void
F_ChangeHandleDestroy(F_ChangeHandle handle)
{
    if(NULL != handle)
    {
        FindCloseChangeNotification(handle);
    }
}
