cmake_minimum_required(VERSION 3.15)
message(STATUS "Found CMake ${CMAKE_VERSION}")
project(Pong3D LANGUAGES C)


add_executable(Pong3D main.c pong3d.c geometry.c renderer.c sound.c synth.c msys.c screens.c tasks.c text.c)

    find_package(SDL2 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(GLEW REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${FREETYPE_INCLUDE_DIRS})
    include_directories(${GLEW_INCLUDE_DIRS})

    target_link_libraries(Pong3D SDL2)
    target_link_libraries(Pong3D ${FREETYPE_LIBRARIES})
    target_link_libraries(Pong3D GLEW)
    target_link_libraries(Pong3D m)
    target_link_libraries(Pong3D GL)


file(COPY fonts DESTINATION .)
