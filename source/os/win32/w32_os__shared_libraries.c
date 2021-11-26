#define SharedLibExport __declspec(dllexport)

Function SharedLib
SharedLibLoad(S8 filename)
{
    SharedLib result;
    
    M_Temp scratch = TC_ScratchGet(NULL, 0);
    
    Assert(sizeof(SharedLib) == sizeof(HMODULE));
    S16 filename_s16 = S16FromS8(scratch.arena, filename);
    HMODULE module = LoadLibraryW(filename_s16.buffer);
    result = (void *)module;
    
    M_TempEnd(&scratch);
    return result;
}

Function void *
SharedLibGet(SharedLib lib, char *symbol)
{
    Assert(NULL != lib);
    void *result;
    HMODULE module = (HMODULE)lib;
    FARPROC fn = GetProcAddress(module, symbol);
    result = (void *)fn;
    return result;
}

Function void
SharedLibUnload(SharedLib lib)
{
    Assert(NULL != lib);
    HMODULE module = (HMODULE)lib;
    FreeLibrary(module);
}
