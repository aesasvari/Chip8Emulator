#include "stack.h"
#include "chip8.h"
#include <assert.h>
#include <stdio.h>

static void stack_check_bounds(struct chip8* chip8)
{
	printf("Stack Pointer Value: %x\n", chip8->registers.stack_pointer);
	printf("Stack Size: %x\n", sizeof(chip8->stack.stack));

	assert(chip8->registers.stack_pointer < sizeof(chip8->stack.stack));
}

void stack_push(struct chip8* chip8, unsigned short val)
{
	chip8->registers.stack_pointer += 0x01;
	stack_check_bounds(chip8);
	chip8->stack.stack[chip8->registers.stack_pointer] = val;
}

unsigned short stack_pop(struct chip8* chip8)
{
	stack_check_bounds(chip8);
	unsigned short val = chip8->stack.stack[chip8->registers.stack_pointer];
	chip8->registers.stack_pointer -= 0x01;
	return val;
}