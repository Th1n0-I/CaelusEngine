cmake_minimum_required(VERSION 3.24)
project(CaelusEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)

include(FetchContent)
FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)

add_executable(CaelusEngine main.cpp)
target_link_libraries(CaelusEngine PRIVATE glfw Vulkan::Vulkan)

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