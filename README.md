# Entry Point - A Template for starting new C projects
This is not intended to be used as a library, separate from the custom code.
It is instead intended to be a template for new projects, containing a selection of code which is common amongst many codebases.


## build system
The entire project is built as a single translation unit, with all source files being included into `main.c`
To build on windows, simply run `build.bat` (ensuring CL.exe is in your PATH).
On linux, `./build.sh`.
The build script always performs a debug and release build, starting with debug so you can test your app quickly.

## codebase structure
The codebase is divided into layers, each of which depend on the layers below it and nothing else

### base layer
This layer contains the most fundamental utilities. It depends on `stb_sprintf.h` and optionally zlib, but nothing else - not even the OS or the C runtime.
It includes:
* misc. macros and helpers
* 'context cracking' (gathering info about CPU architecture, OS, compiler, etc.)
* memory allocators and utilities
* length based strings and associated utility functions and types
* sorting (plagiarised from musl)
* maths functions and vector types
* random number generation
* types for storing times

### os layer
This layer contains basic OS functionality. It depends on the base layer and the OS.
It includes:
* an implicit thread context
* console output
* command line arguments
* date/time and precision time functions
* minimal file io
* shared library loading
* entropy
* threading and synchronisation
* minimal clipboard api (supported formats utf-8 text and lists of filenames)

### graphics layer
This layer include basic rendering functionality to assist with getting something on the screen quickly.
It depends on the base layer, the os layer, the OS itself, `stb_rect_pack.h` and `stb_truetype.h`.
It includes:
* window and graphics context creation
* input event queues
* a render command queue system
* 2D renderer (sprites, text, shapes)

