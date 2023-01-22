#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>

class Graphics {
public:
	Graphics(int width = 240*4, int height = 160*4);
	~Graphics();
	void drawFrame();
private:
	//window stuff
	SDL_Window* _window;
	SDL_Renderer* _renderer;
	SDL_Texture* _windowScreen;

	int _windowWidth, _windowHeight;
	int _windowPosX, _windowPosY;
	float _renderScaleX, _renderScaleY;
};

#endif
