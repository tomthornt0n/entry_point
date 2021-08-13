typedef void *OS_SharedLib;

static OS_SharedLib OS_SharedLibLoad(S8 filename);
static void *OS_SharedLibGet(OS_SharedLib lib, char *symbol);
static void OS_SharedLibUnload(OS_SharedLib lib);
