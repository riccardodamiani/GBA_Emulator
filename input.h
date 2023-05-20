#ifndef	INPUT_H
#define INPUT_H

#include <SDL.h>
#include <array>

struct keyinput_struct {
	uint16_t a : 1,
		b : 1,
		select : 1,
		start : 1,
		right : 1,
		left : 1,
		up : 1,
		down : 1,
		r : 1,
		l : 1,
		not_used : 6;
};

struct joypad_map {
	SDL_Scancode a, b, select, start;
	SDL_Scancode right, left, up, down;
	SDL_Scancode r, l;
};

class Input {
public:
	Input();
	void update();
	void beginNewFrame();
	void setJoypadState(void);
	//returns the state of a key or a mouse button
	bool wasKeyPressed(SDL_Scancode key);
	bool wasKeyReleased(SDL_Scancode key);
	bool isKeyHeld(SDL_Scancode key);
	void saveKeyboardMap();
	bool loadKeyboardMap();
private:
	void keyUpEvent(const SDL_Event& event);
	void keyDownEvent(const SDL_Event& event);
	void getSDLEvent();

	std::array <bool, SDL_Scancode::SDL_NUM_SCANCODES> _heldKeys;
	std::array <bool, SDL_Scancode::SDL_NUM_SCANCODES> _pressedKeys;
	std::array <bool, SDL_Scancode::SDL_NUM_SCANCODES> _releasedKeys;

	keyinput_struct joypad;
	joypad_map keysMap;
};

#endif
