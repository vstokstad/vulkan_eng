@echo off
echo %cd%
echo compiling shaders:
forfiles /P shaders /S /M *.vert /C "cmd /c echo @file"
forfiles /P shaders /S /M *.frag /C "cmd /c echo @file"
forfiles /P shaders /S /M *.vert /C "cmd /c glslc -c @file"
forfiles /P shaders /S /M *.frag /C "cmd /c glslc -c @file"
echo Done! compiled files:
forfiles /P shaders /D +0 /S /M *.spv /C "cmd /c echo @file"