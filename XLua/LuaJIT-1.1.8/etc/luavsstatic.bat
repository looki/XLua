@rem Script to build LuaJIT under "Visual Studio .NET Command Prompt".
@rem Do not run from this directory; run it from the toplevel: etc\luavs.bat .
@rem It creates lua51.dll, lua51.lib and luajit.exe in src.
@rem (contributed by David Manura and Mike Pall)

@setlocal
@set MYCOMPILE=cl /nologo /MT /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE /I . /I ..\dynasm
@set MYLINK=link /nologo
@set MYMT=mt /nologo

cd bin
%MYCOMPILE% ..\src\l*.c
del lua.obj luac.obj
lib /nologo /nodefaultlib /out:lua51.lib l*.obj
rem %MYLINK% /DLL /out:lua51.dll l*.obj
rem if exist lua51.dll.manifest^
rem   %MYMT% -manifest lua51.dll.manifest -outputresource:lua51.dll;2
rem %MYCOMPILE% /DLUA_BUILD_AS_DLL lua.c
rem %MYLINK% /out:luajit.exe lua.obj lua51.lib
rem if exist luajit.exe.manifest^
rem   %MYMT% -manifest luajit.exe.manifest -outputresource:luajit.exe
del *.obj *.manifest
cd ..
