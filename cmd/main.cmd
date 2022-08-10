@echo off

SET includes=
SET links=
SET defines=

echo "clean"
del build\Main.exe

echo "compile"
g++ %includes% -c src\Main.cpp -o bin\main.o -g

echo "build"
g++ bin\main.o %links% -o build\Main.exe -g

echo "obj-clean"
del bin\*.o /Q /F