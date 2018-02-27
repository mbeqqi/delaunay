@echo off
pushd ..\build

SET CompilerArgs=-MD -nologo -Z7 -Od -Gm- -GR- -EHsc
SET LinkerArgs=-link  /SUBSYSTEM:CONSOLE -OUT:triang.exe

cl /F40000000 %CompilerArgs% -I../SDL2/include -I../glew-2.0.0/include ..\code\main.cpp ../SDL2/lib/x86/*.lib opengl32.lib glu32.lib ../glew-2.0.0/lib/release/Win32/glew32.lib  %LinkerArgs%

popd ..\build