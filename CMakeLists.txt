cmake_minimum_required(VERSION 3.5)
message(STATUS "Found CMake ${CMAKE_VERSION}")
project(pong3D LANGUAGES C)


add_executable(pong3D main.c pong3d.c geometry.c renderer.c sound.c synth.c msys.c screens.c tasks.c text.c)

    find_package(SDL2 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(GLEW REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${FREETYPE_INCLUDE_DIRS})
    include_directories(${GLEW_INCLUDE_DIRS})

    target_link_libraries(pong3D ${SDL2_LIBRARIES})
    target_link_libraries(pong3D ${FREETYPE_LIBRARIES})
    target_link_libraries(pong3D ${GLEW_LIBRARIES})
    target_link_libraries(pong3D m)
    target_link_libraries(pong3D gl)


file(COPY fonts DESTINATION .)
