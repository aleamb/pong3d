# Pong3D

Pong game in three dimensions write in C using SDL2 and OpenGL 3, recreation of the pong 3d game by Liquid Media (www.liquid.se/pong).

![Pong3D](screenshot.png)

## Building and running

This program depends on:

* SDL2 lib
* Freetype2 lib
* Glew
* OpenGL libraries (3.2)


### Build on Unix/Linux systems

1. Install dependencies SDL2, Freetype2, Glew and OpenGL 4 using your favorite package manager.

By example, in Debian based systems:

```
# apt-get install libsdl2-dev libglew-dev libfreetype6-dev
```

2. Build
```
make
```

3. Run with
```
./pong3d
```

### Build on Windows with [MSYS2](https://www.msys2.org/)

1. Open msys2 terminal. Make sure you have build tools installed (mingw64, binutils)

2. Install dependencies SDL2, Glew, Freetype2

```
pacman -S mingw64/mingw-w64-x86_64-SDL2 mingw64/mingw-w64-x86_64-glew mingw64/mingw-w64-x86_64-freetype
```
3. make sure that gcc points to /mingw64/bin/gcc with

```
which gcc
```

If not, install mingw-64

```
pacman -S mingw-w64-x86_64-gcc
```

4. Build

```
make -f Makefile.msys2
```

5. Run in msys2 terminal with

```
./pong3d.exe
```


## License

Pong3d is licensed under the MIT license. (http://opensource.org/licenses/MIT)


