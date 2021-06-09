#include "screen.h"
#include <assert.h>
#include <memory.h>
#include <string.h>

static void screen_check_bounds(int x, int y)
{
	assert(x >= 0 && x < CHIP8_WIDTH && y >= 0 && y < CHIP8_HEIGHT);
}

void screen_set(struct chip8_screen* screen, int x, int y)
{
	screen_check_bounds(x, y);
	screen->pixels[y][x] = true;
}

bool screen_is_set(struct chip8_screen* screen, int x, int y)
{
	screen_check_bounds(x, y);
	return screen->pixels[y][x];
}

bool screen_draw_sprite(struct chip8_screen* screen, int x, int y, const char* sprite, int num)
{
	bool collision = false;

	for (int ly = 0; ly < num; ly++)
	{
		char c = sprite[ly];
		for (int lx = 0; lx < 8; lx++)
		{
			if ((c & (0b10000000 >> lx)) == 0) continue;

			if (screen->pixels[(ly + y) % CHIP8_HEIGHT][(lx + x) % CHIP8_WIDTH]) collision = true;

			screen->pixels[(ly + y) % CHIP8_HEIGHT][(lx + x) % CHIP8_WIDTH] ^= true;
		}
	}

	return collision;
}

void screen_clear(struct chip8_screen* screen)
{
	memset(screen->pixels, 0, sizeof(screen->pixels));
}