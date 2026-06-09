# Min required CMAKE version
cmake_minimum_required(VERSION 3.28)


# Project name
include(FetchContent)
project(assignment1)

set(FETCHCONTENT_QUIET OFF)
# Set compilier version (C++ 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set location for findSFML.cmake parent directory.
# If you used 'apt-get install libsfml-dev' you will need to set this
set(CMAKE_MODULE_PATH "/usr/share/SFML/cmake/Modules")

# Tell cmake to find SFML
# Declare which SFML components to use
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(imgui)
# ImGui-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)


# ImGui-SFML
FetchContent_Declare(
    ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(ImGui-SFML)

FetchContent_MakeAvailable(SFML imgui ImGui-SFML)

# Copy 'fonts' folder into the built bin folder
file(GLOB_RECURSE FONTS fonts/*.ttf)
file(INSTALL ${FONTS} DESTINATION ${CMAKE_BINARY_DIR}/fonts)

# Copy config file
file(INSTALL config.txt DESTINATION ${CMAKE_BINARY_DIR}/)

add_executable(assignment1 src/main.cpp)

# Link SFML libraries. Every component used aboved in 'find_package' must be linked here
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        ImGui-SFML::ImGui-SFML
)