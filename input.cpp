#include "input.h"
#include "gba.h"

#include <SDL.h>
#include <iostream>
#include <fstream>

Input::Input() {
	if (!loadKeyboardMap()) {
		keysMap = { SDL_SCANCODE_O, SDL_SCANCODE_P, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_SPACE,
			SDL_SCANCODE_D, SDL_SCANCODE_A, SDL_SCANCODE_W, SDL_SCANCODE_S,
			SDL_SCANCODE_E, SDL_SCANCODE_Q
		};
	}

	for (int i = 0; i < _pressedKeys.size(); i++) {
		_pressedKeys[i] = 0;
		_releasedKeys[i] = 0;
		_heldKeys[i] = 0;
	}
	joypad = {};
}

//update the inputs
void Input::update() {
	beginNewFrame();
	getSDLEvent();
	setJoypadState();
}

void Input::saveKeyboardMap() {
	std::ofstream file("keymap.dat", std::ios::binary);
	file.write((char*)&keysMap, sizeof(keysMap));
	file.close();
}

bool Input::loadKeyboardMap() {
	std::ifstream file("keymap.dat", std::ios::binary);
	if (!file.is_open())
		return false;
	file.read((char*)&keysMap, sizeof(keysMap));
	file.close();
	return true;
}

void Input::beginNewFrame() {
	for (int i = 0; i < _pressedKeys.size(); i++) {
		_pressedKeys[i] = 0;
		_releasedKeys[i] = 0;
	}
	getSDLEvent();

	joypad.left = (isKeyHeld(keysMap.left));
	joypad.right = (isKeyHeld(keysMap.right));
	joypad.up = (isKeyHeld(keysMap.up));
	joypad.down = (isKeyHeld(keysMap.down));
	joypad.a = (isKeyHeld(keysMap.a));
	joypad.b = (isKeyHeld(keysMap.b));
	joypad.select = (isKeyHeld(keysMap.select));
	joypad.start = (isKeyHeld(keysMap.start));
	joypad.r = (isKeyHeld(keysMap.r));
	joypad.l = (isKeyHeld(keysMap.l));

	if (wasKeyReleased(SDL_SCANCODE_F3)) {
		
	}
}

//get a input event and convert it into a sdl event
void Input::getSDLEvent() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			exit(EXIT_SUCCESS);
			return;
		}
		else if (event.type == SDL_KEYDOWN) {	//keyboard input
			if (event.key.repeat == 0) {
				this->keyDownEvent(event);
			}
		}
		else if (event.type == SDL_KEYUP) {
			this->keyUpEvent(event);
		}
	}
}

void Input::setJoypadState(void) {
	GBA::memory.setKeyInput(joypad);
}

//gets called when a key is pressed
void Input::keyDownEvent(const SDL_Event& event) {

	this->_pressedKeys[event.key.keysym.scancode] = true;
	this->_heldKeys[event.key.keysym.scancode] = true;
}

//gets called when a key is released
void Input::keyUpEvent(const SDL_Event& event) {

	this->_releasedKeys[event.key.keysym.scancode] = true;
	this->_heldKeys[event.key.keysym.scancode] = false;
}

//checks if a certain key was pressed during the current frame
bool Input::wasKeyPressed(SDL_Scancode key) {
	return this->_pressedKeys[key];
}

//checks if a certain key was released during the current frame
bool Input::wasKeyReleased(SDL_Scancode key) {
	return this->_releasedKeys[key];
}

//checks if a certain key is currently being held
bool Input::isKeyHeld(SDL_Scancode key) {
	return this->_heldKeys[key];
}