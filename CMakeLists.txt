cmake_minimum_required(VERSION 3.20)

project(3DMotor
    VERSION 1.0.0
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are disabled. Run cmake from a dedicated build directory.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)

include(FetchContent)

# 1. GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# 2. GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# 3. Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
)

FetchContent_MakeAvailable(glfw glm imgui)

add_library(glad_lib STATIC external/glad/src/glad.c)

target_include_directories(glad_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include"
)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include/imgui")
file(COPY "${imgui_SOURCE_DIR}/" DESTINATION "${CMAKE_BINARY_DIR}/include/imgui"
    FILES_MATCHING PATTERN "*.h" PATTERN "*.cpp")
file(COPY "${imgui_SOURCE_DIR}/backends/" DESTINATION "${CMAKE_BINARY_DIR}/include/imgui"
    FILES_MATCHING PATTERN "*.h" PATTERN "*.cpp")

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_lib PUBLIC
    "${CMAKE_BINARY_DIR}/include"
    "${CMAKE_BINARY_DIR}/include/imgui"
)

target_link_libraries(imgui_lib PUBLIC
    glad_lib
    glfw
    OpenGL::GL
)

file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(3DMotor ${ENGINE_SOURCES})

target_include_directories(3DMotor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(3DMotor PRIVATE
    imgui_lib
    glad_lib
    glm::glm
    glfw
    OpenGL::GL
)

if(MSVC)
    target_compile_options(3DMotor PRIVATE
        /W3
        /permissive-
        /utf-8
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET 3DMotor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:3DMotor>/assets"
        COMMENT "Copying assets/ directory to output folder..."
    )
endif()