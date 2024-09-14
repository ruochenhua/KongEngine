cmake_minimum_required(VERSION 3.13)

project(KongEngine)

set(CMAKE_CXX_EXTENSIONS 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
set(SRC_DIR tinyGL/src)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

add_subdirectory(${3RD_PARTY_DIR})

# imgui编译
set(IMGUI_DIR ${3RD_PARTY_DIR}/imgui)
file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp)
file(GLOB IMGUI_BACKEND_SRC ${IMGUI_DIR}/backends/*.cpp)

file(GLOB_RECURSE MAIN_SRC ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE MAIN_HEAD ${SRC_DIR}/*.h)



add_executable(KongEngine ${MAIN_HEAD} ${MAIN_SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})
# include_directories(KongEngine ${3RD_PARTY_DIR})
include_directories(KongEngine tinyGL/src)
include_directories(KongEngine ${3RD_PARTY_DIR})

target_include_directories(KongEngine PRIVATE ${IMGUI_DIR})
target_include_directories(KongEngine PRIVATE ${IMGUI_DIR}/backends)

# link library
target_link_libraries(KongEngine OpenGL::GL)
target_link_libraries(KongEngine glfw)
target_link_libraries(KongEngine glm_static)
target_link_libraries(KongEngine glad)
target_link_libraries(KongEngine assimp)
target_link_libraries(KongEngine yaml-cpp)