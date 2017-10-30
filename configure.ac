#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([pong3d], [1.0], [])

AM_INIT_AUTOMAKE(pong3d, 1.0)

#AC_CONFIG_SRCDIR([tasks.c])
#AC_CONFIG_HEADERS([pong3d.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lGL':
AC_CHECK_LIB([GL], [glClear])
# FIXME: Replace `main' with a function in `-lGLEW':
AC_CHECK_LIB([GLEW], [glewInit])
# FIXME: Replace `main' with a function in `-lSDL2':
AC_CHECK_LIB([SDL2], [SDL_CreateWindow])
# FIXME: Replace `main' with a function in `-lfreetype':
AC_CHECK_LIB([freetype], [FT_Init_FreeType])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [sin])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
