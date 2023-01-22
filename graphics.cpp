#include "graphics.h"
#include "lcd_controller.h"
#include "gba.h"

#include <SDL.h>

Graphics::Graphics(int width, int height) {
	_windowWidth = width;
	_windowHeight = height;
	_renderScaleX = (float)width / 240.0;
	_renderScaleY = (float)height / 160.0;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");		//needed otherwise imgui breaks when resizing the window
	_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _windowWidth, _windowHeight, 0);
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	SDL_SetWindowTitle(this->_window, "GBA Emulator");
	SDL_GetWindowPosition(_window, &_windowPosX, &_windowPosY);
	SDL_SetWindowInputFocus(_window);

	_windowScreen = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 240, 160);
	SDL_SetTextureBlendMode(_windowScreen, SDL_BLENDMODE_NONE);
}

Graphics::~Graphics() {

}

void Graphics::drawFrame() {

	SDL_SetRenderTarget(_renderer, nullptr);
	SDL_SetRenderDrawBlendMode(this->_renderer, SDL_BLENDMODE_NONE);

	//draw the buffer
	const uint32_t* const screen = GBA::lcd_ctl.getBufferToRender();
	int pitch;
	void* pixelBuffer;
	SDL_LockTexture(_windowScreen, nullptr, &pixelBuffer, &pitch);
	/*if (SDL_LockTexture(_windowScreen, nullptr, &pixelBuffer, &pitch) < 0)
		fatal(FATAL_TEXTURE_LOCKING_FAILED, __func__);*/
	memcpy(pixelBuffer, screen, 240 * 160 * 4);
	SDL_UnlockTexture(_windowScreen);
	SDL_SetRenderTarget(_renderer, nullptr);
	SDL_RenderCopy(_renderer, _windowScreen, nullptr, nullptr);

	SDL_RenderPresent(this->_renderer);
}