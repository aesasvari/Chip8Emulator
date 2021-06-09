#include "keyboard.h"
#include <assert.h>

static void keyboard_check_bounds(int key)
{
	assert(key >= 0 && key < CHIP8_KEYS);
}

void keyboard_down(struct chip8_keyboard* keyboard, int key)
{
	keyboard->keyboard[key] = true;
}

void keyboard_up(struct chip8_keyboard* keyboard, int key)
{
	keyboard->keyboard[key] = false;
}

bool keyboard_check_down(struct chip8_keyboard* keyboard, int key)
{
	return keyboard->keyboard[key];
}

int keyboard_map(struct chip8_keyboard* keyboard, char key)
{
	for (int i = 0; i < CHIP8_KEYS; i++)
	{
		if (keyboard->keyboard_map[i] == key) return i;
	}

	return -1;
}

void keyboard_set_map(struct chip8_keyboard* keyboard, const char* map)
{
	keyboard->keyboard_map = map;
}