cmake_minimum_required(VERSION 3.5)

project(3rdparty)

set(CMAKE_CXX_EXTENSIONS 17)
set(3RD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# some thirdparty library
set(GLFW_DIR ${3RD_PARTY_DIR}/glfw)
# include_directories(KongEngine ${GLFW_DIR}/include)

set(GLM_DIR ${3RD_PARTY_DIR}/glm)
# include_directories(KongEngine ${GLM_DIR})

set(JSON_LOADER_DIR ${3RD_PARTY_DIR}/nlohmann)
# include_directories(KongEngine ${JSON_LOADER_DIR})

set(YAML_DIR ${3RD_PARTY_DIR}/yaml)
# include_directories(KongEngine ${YAML_DIR}/include)

set(GLAD_DIR ${3RD_PARTY_DIR}/glad)
# include_directories(KongEngine ${GLAD_DIR}/include)

set(ASSIMP_DIR ${3RD_PARTY_DIR}/assimp)
# include_directories(KongEngine ${ASSIMP_DIR}/include)


# build glfw
add_subdirectory(${GLFW_DIR})

# build glm
add_subdirectory(${GLM_DIR})

# build glad
add_subdirectory(${GLAD_DIR})

# build assimp
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(${ASSIMP_DIR})

# build yaml
set(YAML_CPP_BUILD_TOOLS OFF)
set(YAML_CPP_BUILD_TESTS OFF)
add_subdirectory(${YAML_DIR})

