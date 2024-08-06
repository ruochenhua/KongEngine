cmake_minimum_required(VERSION 3.5)

project(TinyGL)

set(CMAKE_CXX_EXTENSIONS 11)

find_package(OpenGL REQUIRED)
set(SRC_DIR tinyGL/src)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
# aux_source_directory(tinyGL/src SRC_LIST)

# some thirdparty library
set(GLEW_DIR ${3RD_PARTY_DIR}/glew)
set(GLFW_DIR ${3RD_PARTY_DIR}/glfw)
set(GLM_DIR ${3RD_PARTY_DIR}/glm)
set(JSON_LOADER_DIR ${3RD_PARTY_DIR}/nlohmann)
set(YAML_DIR ${3RD_PARTY_DIR}/yaml)
# set(OBJ_LOADER_DIR ${3RD_PARTY_DIR}/objloader)
set(GLAD_DIR ${3RD_PARTY_DIR}/glad)
set(ASSIMP_DIR ${3RD_PARTY_DIR}/assimp)
set(IMGUI_DIR ${3RD_PARTY_DIR}/imgui)
file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp)
file(GLOB IMGUI_BACKEND_SRC ${IMGUI_DIR}/backends/*.cpp)

# build glfw
add_subdirectory(${GLFW_DIR})

# build glm
add_subdirectory(${GLM_DIR})

# build glad
add_subdirectory(${GLAD_DIR})

# build assimp
add_subdirectory(${ASSIMP_DIR})

# build yaml
add_subdirectory(${YAML_DIR})

include_directories(${3RD_PARTY_DIR} ${OBJ_LOADER_DIR})
include_directories(tinyGL/src)
file(GLOB_RECURSE MAIN_SRC ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE MAIN_HEAD ${SRC_DIR}/*.h)

file(GLOB_RECURSE 3RD_PARTY_SRC &)

add_executable(TinyGL ${MAIN_HEAD} ${MAIN_SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})
target_include_directories(TinyGL PRIVATE ${IMGUI_DIR})
target_include_directories(TinyGL PRIVATE ${IMGUI_DIR}/backends)

# link library
target_link_libraries(TinyGL OpenGL::GL)
target_link_libraries(TinyGL glfw)
target_link_libraries(TinyGL glm_static)
target_link_libraries(TinyGL glad)
target_link_libraries(TinyGL assimp)
target_link_libraries(TinyGL yaml-cpp)