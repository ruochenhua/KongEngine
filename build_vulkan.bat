IF NOT EXIST "project_build" MD "project_build"

cd project_build
cmake -DUSE_VULKAN=ON ..
