cmake_minimum_required(VERSION 3.10)
project(DSDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Répertoires
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

# Fichiers sources et headers
file(GLOB_RECURSE SRC_FILES ${SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.hpp)

# Exécutable
add_executable(DSDA ${SRC_FILES} ${HEADER_FILES})

# Répertoires d'inclusion
target_include_directories(DSDA PUBLIC ${INCLUDE_DIR})

# Bibliothèques
# find_package(OpenGL REQUIRED)
# find_package(SDL2 REQUIRED)
# find_package(GLFW3 REQUIRED)

# target_link_libraries(DSDA PRIVATE OpenGL::GL GLEW SDL2 glfw)

# Options de compilation
if (MSVC)
    target_compile_options(DSDA PRIVATE /W3)
else()
    target_compile_options(DSDA PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors)
endif()

# Définir les propriétés des fichiers générés.
set_target_properties(DSDA PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR}
)

# Copier les assets et shaders dans le répertoire de build.
file(COPY ${ASSETS_DIR} DESTINATION ${BINARY_DIR})
file(COPY ${SHADER_DIR} DESTINATION ${BINARY_DIR}/shaders)