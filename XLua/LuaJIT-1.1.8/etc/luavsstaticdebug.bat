@rem Script to build LuaJIT under "Visual Studio .NET Command Prompt".
@rem Do not run from this directory; run it from the toplevel: etc\luavs.bat .
@rem It creates lua51.dll, lua51.lib and luajit.exe in src.
@rem (contributed by David Manura and Mike Pall)

@setlocal
@set MYCOMPILE=cl /nologo /MT /DEBUG:FULL /Z7 /W3 /c /D_CRT_SECURE_NO_DEPRECATE /I . /I ..\dynasm
@set MYLINK=link /nologo
@set MYMT=mt /nologo

cd bin
%MYCOMPILE% ..\src\l*.c ..\src\bit.c
del lua.obj luac.obj
lib /nologo /nodefaultlib /out:lua51d.lib l*.obj bit.obj
del *.obj *.manifest
cd ..

