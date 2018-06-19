cmake_minimum_required(VERSION 3.5)
message(STATUS "Found CMake ${CMAKE_VERSION}")
project(Pong3D)

if (WIN32)
    set(SDL2_DIR "$ENV{SDL2_DIR}")
    set(FREETYPE_DIR "$ENV{FREETYPE_DIR}")
    set(GLEW_DIR "$ENV{GLEW_DIR}")

    if (NOT SDL2_DIR)
        message(FATAL_ERROR "set SDL2_DIR environment variable")
    endif()
    if (NOT FREETYPE_DIR)
        message(FATAL_ERROR "set FREETYPE_DIR environment variable")
    endif()
    if (NOT GLEW_DIR)
        message(FATAL_ERROR "set GLEW_DIR environment variable")
    endif()

    include_directories("${SDL2_DIR}/include")
    include_directories("${FREETYPE_DIR}/include")
    include_directories("${GLEW_DIR}/include")

    add_definitions(-D_USE_MATH_DEFINES)

else()
    find_package(SDL2 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(GLEW REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${FREETYPE_INCLUDE_DIRS})
    include_directories(${GLEW_INCLUDE_DIRS})

endif()

add_executable(pong3d main.c pong3d.c geometry.c renderer.c sound.c synth.c msys.c screens.c tasks.c text.c)


if (WIN32)
    set_target_properties(pong3d PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(pong3d PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(pong3d PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif(WIN32)

if (WIN32)
    target_link_libraries(pong3d "${SDL2_DIR}/lib/x64/SDL2.lib")
    target_link_libraries(pong3d "${FREETYPE_DIR}/win64/freetype.lib")
    target_link_libraries(pong3d "${GLEW_DIR}/lib/Release/x64/glew32.lib")
    target_link_libraries(pong3d opengl32.lib)
else()
    target_link_libraries(pong3d SDL2)
    target_link_libraries(pong3d ${FREETYPE_LIBRARIES})
    target_link_libraries(pong3d GLEW)
    target_link_libraries(pong3d m)
    target_link_libraries(pong3d GL)
endif()

file(COPY fonts DESTINATION .)