cmake_minimum_required(VERSION 3.14)
message(STATUS "Found CMake ${CMAKE_VERSION}")
project(Pong3D LANGUAGES C)


add_executable(Pong3D main.c pong3d.c geometry.c renderer.c sound.c synth.c msys.c screens.c tasks.c text.c)

    find_package(SDL2 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(GLEW REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${FREETYPE_INCLUDE_DIRS})
    include_directories(${GLEW_INCLUDE_DIRS})

    target_link_libraries(Pong3D ${SDL2_LIBRARIES})
    target_link_libraries(Pong3D ${FREETYPE_LIBRARIES})
    target_link_libraries(Pong3D ${GLEW_LIBRARIES})
    target_link_libraries(Pong3D m)
    target_link_libraries(Pong3D gl)


file(COPY fonts DESTINATION .)
