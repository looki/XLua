@rem Script to build LuaJIT under "Visual Studio .NET Command Prompt".
@rem Do not run from this directory; run it from the toplevel: etc\luavs.bat .
@rem It creates lua51.dll, lua51.lib and luajit.exe in src.
@rem (contributed by David Manura and Mike Pall)

@setlocal
@set SOURCES=..\src\l*.c ..\src\bit.c
@set OBJECTS=l*.obj bit.obj
@set MYCOMPILE=cl /nologo /MT /W3 /c /D_CRT_SECURE_NO_DEPRECATE /I . /I ..\dynasm
@set MYCOMPILE_RELEASE=%MYCOMPILE% /O2
@set MYCOMPILE_DEBUG=%MYCOMPILE% /DEBUG:FULL /Z7

cd bin
%MYCOMPILE_RELEASE% %SOURCES%
del lua.obj luac.obj
lib /nologo /nodefaultlib /out:lua51.lib %OBJECTS%
del *.obj *.manifest

%MYCOMPILE_DEBUG% %SOURCES%
del lua.obj luac.obj
lib /nologo /nodefaultlib /out:lua51d.lib %OBJECTS%
del *.obj *.manifest
cd ..
