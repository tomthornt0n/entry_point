@echo off

set app_name=test_app

if not exist build mkdir build
pushd build

echo ~~~~~~~ shader compilation ~~~~~~~~
fxc.exe /nologo /T vs_5_0 /E vs /O3 /WX /Zpc /Ges /Fh w32__vshader.h /Vn d3d11_vshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\source\graphics\win32\w32__shader.hlsl
fxc.exe /nologo /T ps_5_0 /E ps /O3 /WX /Zpc /Ges /Fh w32__pshader.h /Vn d3d11_pshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv ..\source\graphics\win32\w32__shader.hlsl

echo ~~~~~~~~~~~ debug build ~~~~~~~~~~~
set compile_options= /DUNICODE /D_UNICODE /DBuild_ModeDebug=1 /DBuild_EnableAsserts=1
set compile_flags= -nologo /Zi /FC /Od /I../source/
set link_flags= /incremental:no /DEBUG:FULL /subsystem:console
cl.exe %compile_flags% %compile_options% /DApplicationName="%app_name%" ..\source\main.c /link %link_flags%  /out:"%app_name%_debug.exe"

echo ~~~~~~~~~~ release build ~~~~~~~~~~
set compile_options= /DUNICODE /D_UNICODE /DBuild_ModeRelease=1 /DBuild_NoCRT=1 /DBuild_EnableAsserts=0
set compile_flags= -nologo /O2 /FC /I../source/ /GS- /Gs2097152 /utf-8 /Gm- /GR- /EHa- /Zi
set link_flags= /opt:ref /incremental:no /debug /subsystem:windows /STACK:0x100000,0x100000 /NODEFAULTLIB
set link_libraries= advapi32.lib d3d11.lib dxgi.lib dxguid.lib gdi32.lib kernel32.lib shell32.lib Ole32.lib shlwapi.lib user32.lib userenv.lib uuid.lib winmm.lib
cl.exe %compile_flags% %compile_options% /DApplicationName="%app_name%" ..\source\main.c /link %link_flags% %link_libraries% /out:"%app_name%.exe"

popd