cmake_minimum_required(VERSION 3.5)
message(STATUS "Found CMake ${CMAKE_VERSION}")
project(pong3D LANGUAGES C)


add_executable(pong3D main.c pong3d.c geometry.c renderer.c sound.c synth.c msys.c screens.c tasks.c text.c)

target_compile_options(pong3D PRIVATE -std=c99)

	find_package(SDL2 QUIET)
if(SDL2_FOUND)
    set(SDL_LIBS ${SDL2_LIBRARIES})
    set(SDL_INCLUDES "${SDL2_PREFIX}/include")
else()
    message("sdl2 package was not found, looking for libs and headers separately")
    find_library(SDL2 NAMES "SDL2")
    find_path(SDL2_INCLUDE "SDL2/SDL.h")
    set(SDL_LIBS ${SDL2})
    set(SDL_INCLUDES "${SDL2_INCLUDE}")
endif()
message("SDL libs are here: ${SDL_LIBS}")


	find_package(GLEW QUIET)
if(GLEW_FOUND)
    set(GLEW_LIBS ${GLEW_LIBRARIES})
    set(GLEW_INCLUDES ${GLEW_INCLUDE_DIRS})
else()
    message("glew package was not found, looking for libs and headers separately")
    find_library(GLEW NAMES "glew")
    find_path(GLEW_INCLUDE "GL/glew.h")
    set(GLEW_LIBS ${GLEW})
    set(GLEW_INCLUDES ${GLEW_INCLUDE})
endif()
message("glew libs are here: ${GLEW_LIBS}")




    find_package(Freetype REQUIRED)


    include_directories(${SDL2_INCLUDES})
    include_directories(${FREETYPE_INCLUDE_DIRS})
    include_directories(${GLEW_INCLUDES})

    target_link_libraries(pong3D ${SDL2_LIBS})
    target_link_libraries(pong3D ${FREETYPE_LIBRARIES})
    target_link_libraries(pong3D ${GLEW_LIBS})
    target_link_libraries(pong3D m)
    target_link_libraries(pong3D GL)


file(COPY fonts DESTINATION .)
