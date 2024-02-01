@echo off

IF NOT EXIST .\build mkdir .\build
pushd .\build

set CommonCompilerFlags= -Od -MTd -nologo /fp:fast -Gm- -GR- -EHa- -Oi -WX -W4 -wd4127 -wd4311 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7 -Fmwin32_main.map
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib Gdi32.lib winmm.lib opengl32.lib
cl %CommonCompilerFlags%  ..\win32_main.cpp /link %CommonLinkerFlags%

popd
