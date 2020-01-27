# Pong3D

Pong game in three dimensions write in C using SDL2 and OpenGL 3, recreation of the pong 3d game by Liquid Media (www.liquid.se/pong).

![Pong3D](screenshot.png)

## Building and running

This software depends on:

* SDL2 lib
* Freetype2 lib
* Glew
* OpenGL libraries (3.2)


### Build on Unix/Linux systems

1. Install CMake and dependencies SDL2, Freetype2 and Glew using system package manager.

By example, for Debian based systems:

```
# apt-get install cmake libsdl2-dev libglew-dev libfreetype6-dev
```

2. Prepare files to compile

```
mkdir build
```

```
cd build
```

```
cmake ..
```

3. Compile

```
cmake --build .
```

4. Run with

```
./pong3d
```

in build directory.


### Build on Windows with Visual Studio with NuGet

1. Download and install [Visual Studio 2017 Community](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community&rel=15) Make Sure that NuGet is selected when install.
2. Open ${projectDir}\build\windows\vs2017\Pong3D.sln
3. Select solution, open context menu, retarget SDK Version if neccesary and select Recompile.
4. Execute.


### Build on Windows with MSYS2-mingw64

1. Install [MSYS2](https://www.msys2.org/)

2. Open mingw64 terminal, not msys terminal. Mingw64 terminal is on msys2 directory with name mingw64.exe

3. Install git gcc make with pacman package manager.

```
pacman -S git gcc make
```

4. Install dependencies SDL2, Glew, Freetype2

```
pacman -S mingw64/mingw-w64-x86_64-SDL2 mingw64/mingw-w64-x86_64-glew mingw64/mingw-w64-x86_64-freetype
```

5. Clone repository

6. Build with

```
cd pong3d
make -f Makefile.mingw64
```

6. Run with

```
./pong3d.exe
```


## License

Pong3d is licensed under the MIT license. (http://opensource.org/licenses/MIT)
