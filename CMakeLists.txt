cmake_minimum_required(VERSION 3.5)

project(TinyGL)

find_package(OpenGL REQUIRED)
set(SRC_DIR tinyGL/src)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
# aux_source_directory(tinyGL/src SRC_LIST)

# some thirdparty library
set(GLEW_DIR ${3RD_PARTY_DIR}/glew)
set(GLFW_DIR ${3RD_PARTY_DIR}/glfw)
set(GLM_DIR ${3RD_PARTY_DIR}/glm)
set(JSON_LOADER_DIR ${3RD_PARTY_DIR}/nlohmann)
set(OBJ_LOADER_DIR ${3RD_PARTY_DIR}/objloader)
set(GLAD_DIR ${3RD_PARTY_DIR}/glad)
# build glfw
add_subdirectory(${GLFW_DIR})

# build glm
add_subdirectory(${GLM_DIR})

# build glad
# add_library(glad ${GLAD_DIR}/src/glad.c)
# target_link_directories(${GLAD_DIR} PUBLIC ${GLAD_DIR}/include)
# set(CMAKE_XXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
add_subdirectory(${GLAD_DIR})

include_directories(${3RD_PARTY_DIR} ${OBJ_LOADER_DIR})
include_directories(tinyGL/src)
file(GLOB MAIN_SRC ${SRC_DIR}/*.cpp)
file(GLOB MAIN_HEAD ${SRC_DIR}/*.h)

file(GLOB_RECURSE 3RD_PARTY_SRC &)

add_executable(TinyGL ${MAIN_HEAD} ${MAIN_SRC} )

# link library
target_link_libraries(TinyGL OpenGL::GL)
target_link_libraries(TinyGL glfw)
target_link_libraries(TinyGL glm_static)
target_link_libraries(TinyGL glad)