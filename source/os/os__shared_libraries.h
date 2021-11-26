typedef void *SharedLib;

Function SharedLib SharedLibLoad   (S8 filename);
Function void     *SharedLibGet    (SharedLib lib, char *symbol);
Function void      SharedLibUnload (SharedLib lib);
