@ECHO OFF
SetLocal EnableDelayedExpansion

SET CppFilenames=
FOR /R %%f in (*.cpp) do (
    SET CppFilenames=!CppFilenames! %%f
)

SET Assembly=engine
SET CompilerFlags=-g -shared -Wvarargs -Wall -Werror
SET IncludeFlags=-Isource -I%VULKAN_SDK%/Include
SET LinkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib
SET Defines=-DMEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Build %Assembly%..."
clang %CppFilenames% %CompilerFlags% -o ../bin/%Assembly%.dll %Defines% %IncludeFlags% %LinkerFlags%