#include "memory.h"
#include <assert.h>
#include <stdio.h>

static void memory_check_bounds(int index)
{
	assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}

void chip8_memory_set(struct chip8_memory* memory, int index, unsigned char val)
{
	memory_check_bounds(index);
	memory->memory[index] = val;
}

unsigned char chip8_memory_get(struct chip8_memory* memory, int index)
{
	memory_check_bounds(index);
	return memory->memory[index];
}

unsigned char chip8_memory_get_short(struct chip8_memory* memory, int index)
{
	unsigned char byte1 = chip8_memory_get(memory, index);
	unsigned char byte2 = chip8_memory_get(memory, index + 1);
	return byte1 << 8 | byte2;
}