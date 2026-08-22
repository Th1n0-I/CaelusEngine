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