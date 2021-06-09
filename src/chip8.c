#include "chip8.h"
#include "SDL.h"
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

const char default_char_set[] = {
	0xf0, 0x90, 0x90, 0x90, 0xf0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xf0, 0x10, 0xf0, 0x80, 0xf0, //2
	0xf0, 0x10, 0xf0, 0x10, 0xf0, //3
	0x90, 0x90, 0xf0, 0x10, 0x10, //4
	0xf0, 0x80, 0xf0, 0x10, 0xf0, //5
	0xf0, 0x80, 0xf0, 0x90, 0xf0, //6
	0xf0, 0x10, 0x20, 0x40, 0x40, //7
	0xf0, 0x90, 0xf0, 0x90, 0xf0, //8
	0xf0, 0x90, 0xf0, 0x10, 0xf0, //9
	0xf0, 0x90, 0xf0, 0x90, 0x90, //a
	0xe0, 0x90, 0xe0, 0x90, 0xe0, //b
	0xf0, 0x80, 0x80, 0x80, 0xf0, //c
	0xe0, 0x90, 0x90, 0x90, 0xe0, //d
	0xf0, 0x80, 0xf0, 0x80, 0xf0, //e
	0xf0, 0x80, 0xf0, 0x80, 0x80  //f
};

void chip8_init(struct chip8* chip8)
{
	memset(chip8, 0, sizeof(struct chip8));
	memcpy(&chip8->chip8_memory.memory, default_char_set, sizeof(default_char_set));
}

void chip8_load(struct chip8* chip8, const char* buffer, size_t size)
{
	assert(size + CHIP8_PROGRAM_LOAD_ADDR < CHIP8_MEMORY_SIZE);
	memcpy(&chip8->chip8_memory.memory[CHIP8_PROGRAM_LOAD_ADDR], buffer, size);
	chip8->registers.program_counter = CHIP8_PROGRAM_LOAD_ADDR;
}

static unsigned char chip8_wait_key(struct chip8* chip8)
{
	SDL_Event event;
	while (SDL_WaitEvent(&event))
	{
		if (event.type != SDL_KEYDOWN) continue;
		char c = event.key.keysym.sym;
		char chip8_key = keyboard_map(&chip8->keyboard, c);
		if (chip8_key != -1) return chip8_key;
	}
}

static void chip8_execute_eight(struct chip8* chip8, unsigned short opcode)
{
	unsigned char x = (opcode >> 8) & 0x000f;
	unsigned char y = (opcode >> 4) & 0x000f;

	unsigned char last_nibble = opcode & 0x000f;

	unsigned short temp = 0;

	switch(last_nibble)
	{
		//Set Vx = Vy
		case 0x00:
			chip8->registers.V[x] = chip8->registers.V[y];
			break;

		//Set Vx = Vx bitwise or Vy	
		case 0x01:
			chip8->registers.V[x] = chip8->registers.V[x] | chip8->registers.V[y];
			break;

		//Set Vx = Vx bitwise and Vy
		case 0x02:
			chip8->registers.V[x] = chip8->registers.V[x] & chip8->registers.V[y];
			break;

		//Set Vx = Vx bitwise xor Vy
		case 0x03:
			chip8->registers.V[x] = chip8->registers.V[x] ^ chip8->registers.V[y];
			break;

		//Set Vx = Vx + Vy, set VF = carry
		case 0x04:
			chip8->registers.V[15] = 0;
			temp = chip8->registers.V[x] + chip8->registers.V[y];
			if (temp > 0xff) chip8->registers.V[15] = 1;

			chip8->registers.V[x] = temp;
			break;

		//Set Vx = Vx -Vy, set VF = Not Borrow
		case 0x05:
			chip8->registers.V[15] = 0;
			if (chip8->registers.V[x] > chip8->registers.V[y]) chip8->registers.V[15] = 1;

			chip8->registers.V[x] = chip8->registers.V[x] - chip8->registers.V[y];
			break;

		//Set VF = 1 if LSB of Vx == 1
		case 0x06:
			chip8->registers.V[15] = chip8->registers.V[x] & 0x01;
			chip8->registers.V[x] = chip8->registers.V[x] / 2;
			break;

		//Set Vx = Vy - Vx, set VF = Not Borrow
		case 0x07:
			chip8->registers.V[15] = 0;
			if (chip8->registers.V[x] > chip8->registers.V[y]) chip8->registers.V[15] = 1;

			chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
			break;

		//Set Vx = Vx * 2, set VF = 1 if MSB of Vx = 1 otherwise 0
		case 0x0E:
			chip8->registers.V[15] = chip8->registers.V[x] & 0x80;
			chip8->registers.V[x] = chip8->registers.V[x] * 2;
			break;
	}
}

static void chip8_execute_f(struct chip8* chip8, unsigned short opcode)
{
	unsigned char x = (opcode >> 8) & 0x000f;

	switch (opcode & 0x00ff)
	{
		//Set Vx = delay timer
		case 0x07:
			chip8->registers.V[x] = chip8->registers.delay_timer;
			break;

		//Wait for key press and store in Vx
		case 0x0a:
			chip8->registers.V[x] = chip8_wait_key(chip8);
			break;

		//Set delay timer = Vx
		case 0x15:
			chip8->registers.delay_timer = chip8->registers.V[x];
			break;

		//Set sound timer = Vx
		case 0x18:
			chip8->registers.sound_timer = chip8->registers.V[x];
			break;

		//Set I = I + Vx
		case 0x1e:
			chip8->registers.I = chip8->registers.I + chip8->registers.V[x];
			break;
		
		//Set I = location of sprite for digit Vx
		case 0x29:
			chip8->registers.I = chip8->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEIGHT;
			break;

		//Set I, I+1, I+2 = BCD representation of Vx
		case 0x33:
			{
				unsigned char hundreds = chip8->registers.V[x]/100;
				unsigned char tens = (chip8->registers.V[x]/10) % 10;
				unsigned char units = chip8->registers.V[x] % 10;

				chip8_memory_set(&chip8->chip8_memory, chip8->registers.I, hundreds);
				chip8_memory_set(&chip8->chip8_memory, chip8->registers.I+1, tens);
				chip8_memory_set(&chip8->chip8_memory, chip8->registers.I+2, units);
			}
			break;

		//Store registers V0 to Vx in memory starting at I
		case 0x55:
			for (int i = 0; i <= x; i++)
			{
				chip8_memory_set(&chip8->chip8_memory, chip8->registers.I+i, chip8->registers.V[i]);
			}
			break;

		//Read registers V0 to Vx from memory
		case 0x65:
			for (int i = 0; i <= x; i++)
			{
				chip8->registers.V[i] = chip8_memory_get(&chip8->chip8_memory, chip8->registers.I+i);
			}
			break;
	}
}

static void chip8_execute_extended(struct chip8* chip8, unsigned short opcode)
{
	unsigned short nnn = opcode & 0x0fff;
	unsigned char kk = opcode & 0x00ff;
	unsigned char n = opcode & 0x000f;

	unsigned char x = (opcode >> 8) & 0x000f;
	unsigned char y = (opcode >> 4) & 0x000f;

	switch (opcode & 0xf000)
	{
		//Jump to nnn
		case 0x1000:
			chip8->registers.program_counter = nnn;
			break;
		
		//Call subroutine at nnn
		case 0x2000:
			stack_push(chip8, chip8->registers.program_counter);
			chip8->registers.program_counter = nnn;
			break;
		
		//Skip next instruction if Vx == kk
		case 0x3000:
			if (chip8->registers.V[x] == kk) chip8->registers.program_counter += 2;
			break;

		//Skip next instruction if Vx != kk
		case 0x4000:
			if (chip8->registers.V[x] != kk) chip8->registers.program_counter += 2;
			break;
		
		//Skip instruction if Vx == Vy
		case 0x5000:
			if (chip8->registers.V[x] == chip8->registers.V[y]) chip8->registers.program_counter += 2;
			break;

		//Put register kk into Vx
		case 0x6000:
			chip8->registers.V[x] = kk;
			break;
		
		//Set Vx = Vx + kk
		case 0x7000:
			chip8->registers.V[x] = chip8->registers.V[x] + kk;
			break;
		
		case 0x8000:
			chip8_execute_eight(chip8, opcode);
			break;

		//Skip instruction if Vx != Vy
		case 0x9000:
			if (chip8->registers.V[x] != chip8->registers.V[y]) chip8->registers.program_counter += 2;
			break;

		//Set I register to nnn
		case 0xA000:
			chip8->registers.I = nnn;
			break;

		//Jump to location nnn + V0
		case 0xB000:
			chip8->registers.program_counter = chip8->registers.V[0] + nnn;
			break;

		//Set Vx = random byte bitwise and kk
		case 0xC000:
			srand(clock());
			chip8->registers.V[x] = (rand() % 255) & kk;
			break;

		//Draw sprite to screen
		case 0xD000:
			{
			const char* sprite = &chip8->chip8_memory.memory[chip8->registers.I];
			chip8->registers.V[15] = screen_draw_sprite(&chip8->screen, chip8->registers.V[x], chip8->registers.V[y], sprite, n);
			}
			break;

		
		case 0xE000:
			{
				switch (opcode & 0x00ff)
				{
					//Skip instruction if key with value Vx is pressed
					case 0x9e:
						if (keyboard_check_down(&chip8->keyboard, chip8->registers.V[x])) chip8->registers.program_counter += 2;
						break;
					
					//Skip instruction if key with value Vx is not pressed
					case 0xa1:
						if (!keyboard_check_down(&chip8->keyboard, chip8->registers.V[x])) chip8->registers.program_counter += 2;
						break;
				}
			}
			break;

		case 0xF000:
			chip8_execute_f(chip8, opcode);
			break;

	}
}

void chip8_execute(struct chip8* chip8, unsigned short opcode)
{
	switch (opcode)
	{
		//Clear display
		case 0x00E0:
			screen_clear(&chip8->screen);
			break;

		//Return from subroutine
		case 0x00EE:
			chip8->registers.program_counter = stack_pop(chip8);
			break;

		default:
			chip8_execute_extended(chip8, opcode);
	}
}