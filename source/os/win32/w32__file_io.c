static uint64_t
W32_F_SizeGet(HANDLE file_handle)
{
    DWORD hi_size, lo_size;
    lo_size = GetFileSize(file_handle, &hi_size);
    uint64_t result = W32_U64FromHiAndLoWords(hi_size, lo_size);
    return result;
}

static S8
OS_F_ReadEntire(M_Arena *arena, S8 filename)
{
    S8 result = {0};
    
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 filename_s16 = S16FromS8(scratch.arena, filename);
        HANDLE file_handle = CreateFileW(filename_s16.buffer,
                                         GENERIC_READ,
                                         0, 0,
                                         OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL,
                                         0);
        if(INVALID_HANDLE_VALUE != file_handle)
        {
            size_t n_total_bytes_to_read = W32_F_SizeGet(file_handle);
            
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
    }
    
    return result;
}

static Bool
OS_F_WriteEntire(S8 filename, S8 data)
{
    Bool success = True;
    
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
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
    }
    
    return success;
}

static DataAccessFlags
W32_F_PropertiesFlagsFromFileAttribs(DWORD attribs)
{
    OS_F_PropertiesFlags result = OS_F_PropertiesFlags_Exists;
    if(attribs & FILE_ATTRIBUTE_DIRECTORY)
    {
        result |= OS_F_PropertiesFlags_IsDirectory;
    }
    if(attribs & FILE_ATTRIBUTE_HIDDEN)
    {
        result |= OS_F_PropertiesFlags_Hidden;
    }
    return result;
}

static DataAccessFlags
W32_DataAccessFlagsFromFileAttribs(DWORD attribs)
{
    DataAccessFlags result = DataAccessFlags_Read | DataAccessFlags_Execute;
    if(!(attribs & FILE_ATTRIBUTE_READONLY))
    {
        result |= DataAccessFlags_Write;
    }
    return result;
}

static OS_F_Properties
OS_F_PropertiesGet(S8 filename)
{
    OS_F_Properties result = {0};
    
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 filename_s16 = S16FromS8(scratch.arena, filename);
        
        WIN32_FILE_ATTRIBUTE_DATA attribs = {0};
        if(GetFileAttributesExW(filename_s16.buffer, GetFileExInfoStandard, &attribs))
        {
            result.flags = W32_F_PropertiesFlagsFromFileAttribs(attribs.dwFileAttributes);
            result.size = W32_U64FromHiAndLoWords(attribs.nFileSizeHigh, attribs.nFileSizeLow);
            result.access_flags = W32_DataAccessFlagsFromFileAttribs(attribs.dwFileAttributes);
            result.creation_time = W32_T_DenseTimeFromFileTime(attribs.ftCreationTime);
            result.access_time = W32_T_DenseTimeFromFileTime(attribs.ftLastAccessTime);
            result.write_time = W32_T_DenseTimeFromFileTime(attribs.ftLastWriteTime);
        }
    }
    
    return result;
}

static Bool
OS_F_Delete(S8 filename)
{
    Bool success;
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 filename_s16 = S16FromS8(scratch.arena, filename);
        success = DeleteFileW(filename_s16.buffer);
    }
    return success;
}

static Bool
OS_F_Move(S8 filename, S8 new_filename)
{
    Bool success;
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 filename_s16 = S16FromS8(scratch.arena, filename);
        S16 new_filename_s16 = S16FromS8(scratch.arena, new_filename);
        success = MoveFileW(filename_s16.buffer, new_filename_s16.buffer);
    }
    return success;
}

static Bool
OS_F_DirectoryMake(S8 filename)
{
    Bool success;
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 filename_s16 = S16FromS8(scratch.arena, filename);
        success = CreateDirectoryW(filename_s16.buffer, 0);
    }
    return success;
}

static Bool
OS_F_DirectoryDelete(S8 filename)
{
    Bool success;
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 filename_s16 = S16FromS8(scratch.arena, filename);
        success = DeleteDirectoryW(filename_s16.buffer);
    }
    return success;
}

typedef union
{
    OS_F_Iterator iter;
    struct
    {
        OS_F_Iterator _;
        HANDLE handle;
        WIN32_FIND_DATAW find_data;
        Bool is_done;
    };
} W32_FileIterator;

static OS_F_Iterator *
OS_F_IteratorMake(M_Arena *arena, S8 directory)
{
    W32_FileIterator *result = M_ArenaPush(arena, sizeof(*result));
    
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 directory_s16 = S16FromS8(scratch.arena, S8FromFmt(scratch.arena, "%.*s\\*", Unravel(directory)));
        result->handle = FindFirstFileW(directory_s16.buffer, &result->find_data);
    }
    return (OS_F_Iterator *)result;
}

static Bool
OS_F_IteratorNext(M_Arena *arena, OS_F_Iterator *iter)
{
    Bool result = False;
    
    W32_FileIterator *_iter = (W32_FileIterator *)iter;
    if(_iter->handle != NULL &&
       _iter->handle != INVALID_HANDLE_VALUE)
    {
        while(!_iter->is_done)
        {
            wchar_t *file_name = _iter->find_data.cFileName;
            Bool should_emit;
            {
                // NOTE(tbt): ommit the directories '.' and '..'
                Bool is_dot_or_dot_dot = (file_name[0] == '.' && (file_name[1] == '\0' || (file_name[1] == '.' && file_name[2] == '\0')));
                should_emit = !is_dot_or_dot_dot;
            }
            
            if(should_emit)
            {
                iter->current_name = S8FromS16(arena, CStringAsS16(file_name));
                iter->current_properties.size = W32_U64FromHiAndLoWords(_iter->find_data.nFileSizeHigh, _iter->find_data.nFileSizeLow);
                iter->current_properties.flags = W32_F_PropertiesFlagsFromFileAttribs(_iter->find_data.dwFileAttributes);
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
            
            if(should_emit)
            {
                break;
            }
        }
    }
    
    return result;
}

static void
OS_F_IteratorEnd(OS_F_Iterator *iter)
{
    W32_FileIterator *_iter = (W32_FileIterator *)iter;
    if(_iter->handle != NULL &&
       _iter->handle != INVALID_HANDLE_VALUE)
    {
        FindClose(_iter->handle);
        _iter->handle = NULL;
    }
}
