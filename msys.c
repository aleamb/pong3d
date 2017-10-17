#include "msys.h"
#include <SDL2/SDL.h>

SDL_Window* window;
SDL_GLContext mainContext;
SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;


int sys_init_video(int width, int height) {

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

int sys_init_sound(int sample_freq) {
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf( stderr, "Sound initialization failed: %s\n", SDL_GetError( ) );
		return -1;
	}


	want.freq = sample_freq;
	want.format = AUDIO_F32SYS;
	want.channels = 1;
	want.samples = 2048;
	want.callback = NULL;

	dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (dev == 0) {
		fprintf( stderr, "Failed opening audio device: %s\n", SDL_GetError( ) );
		return -1;
	} else if (have.format != want.format) {
		fprintf( stderr, "Failed getting sample format (AUDIO_F32SYS)\n");
		return -1;
	}
	//
	SDL_PauseAudioDevice(dev, 0);
}

void sys_play_sound(void *samples, int data_size) {
	SDL_QueueAudio(dev, samples, data_size);
}

void sys_dispose_screen() {
	SDL_DestroyWindow(window);
}

void sys_dispose_audio() {
	SDL_CloseAudioDevice(dev);
	SDL_CloseAudio();
}

void sys_quit(int errolevel) {
	SDL_Quit();
}

