#include "display.h"
#include <SDL2/sdl.h>

SDL_Window* window;
SDL_GLContext mainContext;

int init_screen(int width, int height) {

  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
    return -1;
  }
	window = SDL_CreateWindow("Pong 3D",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_OPENGL);

  if (!window) {
    fprintf( stderr, "Window set failed: %s\n", SDL_GetError( ) );
    return -1;
  }

  mainContext = SDL_GL_CreateContext(window);

  if (!mainContext) {
    fprintf( stderr, "GL context creation failed: %s\n", SDL_GetError( ) );
    return -1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  return 0;
}
