#define OS_SharedLibExport __declspec(dllexport)

static OS_SharedLib
OS_SharedLibLoad(S8 filename)
{
    OS_SharedLib result;
    M_Temp scratch;
    OS_TC_ScratchMem(scratch, NULL, 0)
    {
        OS_Assert(sizeof(OS_SharedLib) == sizeof(HMODULE));
        S16 filename_s16 = S16FromS8(scratch.arena, filename);
        HMODULE module = LoadLibraryW(filename_s16.buffer);
        result = (void *)module;
    }
    return result;
}

static void *
OS_SharedLibGet(OS_SharedLib lib, char *symbol)
{
    OS_Assert(NULL != lib);
    void *result;
    HMODULE module = (HMODULE)lib;
    FARPROC proc = GetProcAddress(module, symbol);
    result = (void *)proc;
    return result;
}

static void
OS_SharedLibUnload(OS_SharedLib lib)
{
    OS_Assert(NULL != lib);
    HMODULE module = (HMODULE)lib;
    FreeLibrary(module);
}