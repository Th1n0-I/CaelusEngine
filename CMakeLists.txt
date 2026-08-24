cmake_minimum_required(VERSION 3.24)
project(CaelusEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)

include(FetchContent)
FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.9b-docking)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

add_executable(CaelusEngine main.cpp
        Renderer.cpp
        Renderer.h
        Math/Math.h)
target_link_libraries(CaelusEngine PRIVATE glfw Vulkan::Vulkan imgui)

find_program(GLSLC glslc HINTS "$ENV{VULKAN_SDK}/Bin")

function(add_shader TARGET NAME)
    set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${NAME})
    set(OUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/${NAME}.spv)
    add_custom_command(
            OUTPUT ${OUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
            COMMAND ${GLSLC} ${SRC} -o ${OUT}
            DEPENDS ${SRC}
            COMMENT "Compiling ${NAME}")
    target_sources(${TARGET} PRIVATE ${OUT})
endfunction()

add_shader(CaelusEngine triangle.vert)
add_shader(CaelusEngine triangle.frag)