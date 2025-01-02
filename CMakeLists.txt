cmake_minimum_required(VERSION 3.10)
project(DSDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Répertoires
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/src/shaders)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

# Fichiers sources et en-tête.
file(GLOB_RECURSE SRC_FILES ${SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.hpp)

# Exécutable
add_executable(DSDA ${SRC_FILES} ${HEADER_FILES})

# Ajouter les répertoires d'inclusion
target_include_directories(DSDA PUBLIC ${INCLUDE_DIR})

# Bibliothèques
add_subdirectory(glimac)
target_link_libraries(DSDA glimac)
target_include_directories(DSDA PUBLIC glimac)
target_include_directories(DSDA PUBLIC glimac/third-party/glad/include)

# Options de compilation
if (MSVC)
    target_compile_options(DSDA PRIVATE /W3)
else()
    target_compile_options(DSDA PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors)
endif()

# Propriétés des fichiers générés
set_target_properties(DSDA PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR}
)

# Copier assets et shaders dans build
file(COPY ${ASSETS_DIR} DESTINATION ${BINARY_DIR})
file(COPY ${SHADER_DIR} DESTINATION ${BINARY_DIR})