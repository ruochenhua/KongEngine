@echo off
setlocal enabledelayedexpansion

rem 启用ANSI颜色支持（仅Windows 10+有效）
reg add HKCU\Console /v VirtualTerminalLevel /t REG_DWORD /d 0x1 /f >nul 2>&1

set SHADER_DIR=resource\shader

rem 编译顶点着色器
for /R "%SHADER_DIR%" %%f in (*.vulkan.vert) do (
    %VULKAN_SDK%\Bin\glslc.exe "%%f" -o "%%~dpnf.vert.spv" 2>nul
    if !errorlevel! neq 0 (
        echo [ERROR] Failed to compile %%f
        %VULKAN_SDK%\Bin\glslc.exe "%%f" -o "%%~dpnf.vert.spv"
        echo Press any key to continue...
        pause >nul
    ) 
)

rem 编译片段着色器
for /R "%SHADER_DIR%" %%f in (*.vulkan.frag) do (
    %VULKAN_SDK%\Bin\glslc.exe "%%f" -o "%%~dpnf.frag.spv" 2>nul
    if !errorlevel! neq 0 (
        echo [ERROR] Failed to compile %%f
        %VULKAN_SDK%\Bin\glslc.exe "%%f" -o "%%~dpnf.frag.spv"
        echo Press any key to continue...
        pause >nul
    )
)

echo All shaders compiled successfully
pause