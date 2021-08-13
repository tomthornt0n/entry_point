@echo off

set app_name=app
set compile_options= /DUNICODE /DApplicationName=%app_name% /DBuild_ModeDebug=1 /DBuild_NoCRT=1 /DBuild_EnableAsserts=1
set compile_flags= -nologo /Zi /FC /I../source/ /GS- /Gs2097152 /utf-8
set link_libs= kernel32.lib gdi32.lib user32.lib winmm.lib advapi32.lib
set link_flags= /opt:ref /incremental:no /debug /subsystem:windows /STACK:0x100000,0x100000

if not exist build mkdir build
pushd build
cl %compile_flags% %compile_options% ..\source\main.c /link %link_libs% %link_flags% /out:%app_name%.exe

popd