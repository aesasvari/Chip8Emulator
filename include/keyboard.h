#ifndef CHIP8KEYBOARD_H
#define CHIP8KEYBOARD_H

#include <stdbool.h>
#include "config.h"

struct chip8_keyboard
{
	bool keyboard[CHIP8_KEYS];
	const char* keyboard_map;
};

void keyboard_down(struct chip8_keyboard* keyboard, int key);
void keyboard_up(struct chip8_keyboard* keyboard, int key);
bool keyboard_check_down(struct chip8_keyboard* keyboard, int key);
int keyboard_map(struct chip8_keyboard* keyboard, char key);
void keyboard_set_map(struct chip8_keyboard* keyboard, const char* map);

#endif