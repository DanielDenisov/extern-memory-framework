cmake_minimum_required(VERSION 4.1)
project(externMemoryFramework)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl)
pkg_check_modules(EGL REQUIRED egl)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/imgui")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

set(PROJECT_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        main.cpp
        config.h
        memory/memory.h
        memory/memory.cpp
        util/targetStructs.h
        draw/Window.h
        draw/protocols/wlr-layer-shell-protocol.c
)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_executable(externMemoryFramework ${PROJECT_SOURCES})

target_include_directories(externMemoryFramework PRIVATE
        ${WAYLAND_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
)

target_link_libraries(externMemoryFramework PRIVATE
        glfw
        OpenGL::GL
        ${WAYLAND_LIBRARIES}
        ${EGL_LIBRARIES}
)