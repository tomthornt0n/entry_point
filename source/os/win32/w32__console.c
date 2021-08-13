static HANDLE w32_console_handle = INVALID_HANDLE_VALUE;

static void
W32_ConsoleInit(void)
{
    FreeConsole();
    AllocConsole();
    SetConsoleCP(CP_UTF8);
    w32_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // TODO(tbt): setup stdout handle to use ANSI escape sequence things
}

static void
OS_ConsoleOutputS8(S8 string)
{
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        S16 string_s16 = S16FromS8(scratch.arena, string);
        WriteConsoleW(w32_console_handle, string_s16.buffer, string_s16.len, NULL, NULL);
    }
}

static void
OS_ConsoleOutputS16(S16 string)
{
    WriteConsoleW(w32_console_handle, string.buffer, string.len, NULL, NULL);
}
