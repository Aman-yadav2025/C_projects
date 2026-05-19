#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdio.h>
#include <stdint.h>

#define MEMORY_SIZE 4096 // 4096 bytes of memory because the original chip-8 harware is limited to 4k memory
#define REGISTER_COUNT 16 // 16 registers because the jumping bytes are 4 and hence can also be used to store 16 values (0-15)
#define STACK_SIZE 16 // same logic here we store where to return after a subroutine call and the jumping bytes are 4 so we can store 16 return addresses (0-15)
#define DISPLAY_HEIGHT 32 // display size is determined by video memory which was 256 bytes back then
#define DISPLAY_WIDTH 64

typedef struct{
    uint8_t memory[MEMORY_SIZE]; // 4K memory
    uint8_t V[REGISTER_COUNT]; // 16 registers
    uint16_t I; // Index register
    uint16_t pc; // Program counter

    uint8_t delay_timer; // Delay timer for 60Hz updates
    uint8_t sound_timer; // Sound timer for 60Hz updates

    uint16_t stack[STACK_SIZE];
    uint16_t sp; // Stack pointer

    uint8_t display[DISPLAY_HEIGHT*DISPLAY_WIDTH]; // Monochrome display buffer
    uint8_t keypad[16]; // Keypad state
} Chip8;

void Chip8_init(Chip8* chip8);

int Chip8_ROM_Loader(Chip8* chip8, const char* filename);

void Chip8_cycle(Chip8* chip8)


#endif