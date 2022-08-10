@echo off

SET includes=-Iinc -Isrc -I%VULKAN_SDK%\Include
SET links=-L%VULKAN_SDK%\Lib -lvulkan-1 -luser32
SET defines=

echo "clean"
del build\Main.exe

echo "compile"
g++ %includes% -c src\platform\win32_platform.cpp -o bin\win32_platform.o -g

echo "build"
g++ bin\win32_platform.o %links% -o build\Main.exe -g

echo "obj-clean"
del bin\*.o /Q /F