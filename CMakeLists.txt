cmake_minimum_required(VERSION 3.13)

project(KongEngine)

set(CMAKE_CXX_EXTENSIONS 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenGL
find_package(OpenGL REQUIRED)

# 查找Vulkan
find_package(Vulkan QUIET)

if(Vulkan_FOUND AND USE_VULKAN)
	add_definitions(-DRENDER_IN_VULKAN)
endif()


set(SRC_DIR tinyGL/src)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)

add_subdirectory(${3RD_PARTY_DIR})

# imgui编译
# if(Vulkan_FOUND AND USE_VULKAN)
# else()
set(IMGUI_DIR ${3RD_PARTY_DIR}/imgui)
file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp)
file(GLOB IMGUI_BACKEND_SRC ${IMGUI_DIR}/backends/*.cpp)
# endif()

file(GLOB_RECURSE MAIN_SRC ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE MAIN_HEAD ${SRC_DIR}/*.h ${SRC_DIR}/*.hpp)

# if(Vulkan_FOUND AND USE_VULKAN)
# add_executable(KongEngine ${MAIN_HEAD} ${MAIN_SRC})
# else()
add_executable(KongEngine ${MAIN_HEAD} ${MAIN_SRC} ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})

target_include_directories(KongEngine PRIVATE ${IMGUI_DIR})
target_include_directories(KongEngine PRIVATE ${IMGUI_DIR}/backends)
# endif()

# include_directories(KongEngine ${3RD_PARTY_DIR})
include_directories(KongEngine tinyGL/src)
include_directories(KongEngine ${3RD_PARTY_DIR})


# link library
target_link_libraries(KongEngine OpenGL::GL)
target_link_libraries(KongEngine glfw)
target_link_libraries(KongEngine glm_static)
target_link_libraries(KongEngine glad)
target_link_libraries(KongEngine assimp)
target_link_libraries(KongEngine yaml-cpp)

if(Vulkan_FOUND AND USE_VULKAN)
	target_link_libraries(KongEngine Vulkan::Vulkan)
endif()