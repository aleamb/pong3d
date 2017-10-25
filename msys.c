#include "msys.h"
#include <SDL2/SDL.h>

SDL_Window* window;
SDL_GLContext mainContext;
SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;

int video_initialized = 0;
int gl_initialized = 0;
int sound_initialized = 0;

static void format_event(SDL_Event* event, SysEvent* sysEvent);

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

	video_initialized = 1;
	mainContext = SDL_GL_CreateContext(window);

	if (!mainContext) {
		fprintf( stderr, "GL context creation failed: %s\n", SDL_GetError( ) );
		return -1;
	}
	gl_initialized = 1;

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
		SDL_CloseAudioDevice(dev);
		fprintf( stderr, "Failed getting sample format (AUDIO_F32SYS)\n");
		return -1;
	}
	sound_initialized = 1;
	SDL_PauseAudioDevice(dev, 0);
	return 0;
}

void sys_play_sound(void *samples, int data_size) {
	SDL_QueueAudio(dev, samples, data_size);
}

void sys_dispose_video() {
	if (gl_initialized) {
		SDL_GL_DeleteContext(mainContext);
	}
	if (video_initialized) {
		SDL_DestroyWindow(window);
	}
}

void sys_dispose_audio() {
	if (sound_initialized) {
		SDL_CloseAudioDevice(dev);
		SDL_CloseAudio();
	}
}

void sys_quit(int errolevel) {
	SDL_Quit();
}

unsigned int sys_get_ticks() {
	return SDL_GetTicks();
}

int sys_wait(SysEvent* sysEvent, unsigned int milis) {
	int hasEvent = 0;
	SDL_Event event;
	if (SDL_PollEvent(&event)) {
		format_event(&event, sysEvent);
		hasEvent = 1;
	} else if (SDL_WaitEventTimeout(&event, milis)) {
		format_event(&event, sysEvent);
		hasEvent = 1;
	}
	return hasEvent;

}

void sys_swap_buffers() {
	SDL_GL_SwapWindow(window);
}

void sys_mouse_center(int width, int height) {
	SDL_WarpMouseInWindow(window, width >> 1, height >> 1);
}
void sys_show_cursor(int show) {
	//	SDL_ShowCursor(show ? SDL_TRUE : SDL_FALSE);
}

static void format_event(SDL_Event* event, SysEvent* sysEvent) {
	switch (event->type) {
		case SDL_QUIT:
			sysEvent->type = CLOSE;
			break;
		case SDL_MOUSEMOTION:
			sysEvent->type = MOUSEMOTION;
			break;
		case SDL_MOUSEBUTTONUP:
			if (event->button.button == SDL_BUTTON_LEFT) {
				sysEvent->type = MOUSELBUTTONUP;
			}
			break;
		case SDL_KEYDOWN:
			if (event->key.keysym.sym == SDLK_ESCAPE) {
				sysEvent->type = CLOSE;
			}
			break;
	}
	sysEvent->x = event->motion.x;
	sysEvent->y = event->motion.y;

}


