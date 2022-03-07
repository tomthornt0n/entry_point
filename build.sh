
pushd build > /dev/null

app_name="example_app"

echo compiling $app_name

TIMEFORMAT="done in %Rs"

echo ~~~~~~~~~~~ debug build ~~~~~~~~~~~
compile_optns="-D Build_ModeDebug=1 -D Build_EnableAsserts=1 -D Build_UseSSE3=1 -D ApplicationName=$app_name -D Build_ZLib=1"
compile_flags='-g -O0 -I../source/ -msse4.1 -ldl -lpthread -lGL -lxcb -lxcb-icccm -lX11 -lX11-xcb -lm -lz'
time gcc $compile_optns $compile_flags ../source/main.c -o "$app_name"_debug

echo ~~~~~~~~~~ release build ~~~~~~~~~~
compile_optns="-D Build_ModeRelease=1 -D Build_UseSSE3=1 -D ApplicationName=$app_name -D Build_ZLib=1"
compile_flags='-O2 -I../source/ -msse4.1 -ldl -lpthread -lGL -lxcb -lxcb-icccm -lX11 -lX11-xcb -lm -lz'
time gcc $compile_optns $compile_flags ../source/main.c -o $app_name

popd > /dev/null
