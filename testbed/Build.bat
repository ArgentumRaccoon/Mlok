@ECHO OFF
SetLocal EnableDelayedExpansion

SET CppFilenames=
FOR /R %%f in (*.cpp) do (
    SET CppFilenames=!CppFilenames! %%f
)

REM echo "Files:" %CppFilenames%

SET Assembly=testbed
SET CompilerFlags=-g
SET IncludeFlags=-Isource -I../engine/source/
SET LinkerFlags=-L../bin/ -lengine.lib
SET Defines=-D_DEBUG -DMIMPORT

ECHO "Building %Assembly%%..."
ECHO "clang %CppFilenames% %CompilerFlags% -o ../bin/%Assembly%.exe %Defines% %IncludeFlags% %LinkerFlags%"
clang %CppFilenames% %CompilerFlags% -o ../bin/%Assembly%.exe %Defines% %IncludeFlags% %LinkerFlags%