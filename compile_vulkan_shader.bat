set SHADER_DIR=resource\shader


for /R "%SHADER_DIR%" %%f in (*.vulkan.vert) do (
    %VULKAN_SDK%\Bin\glslc.exe "%%f" -o "%%~dpnf.vert.spv"
)

REM 编译所有 .frag 文件 (包括所有子文件夹)
for /R "%SHADER_DIR%" %%f in (*.vulkan.frag) do (
    %VULKAN_SDK%\Bin\glslc.exe "%%f" -o "%%~dpnf.frag.spv"
)

pause
