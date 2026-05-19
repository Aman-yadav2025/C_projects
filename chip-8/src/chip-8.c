#include "chip-8.h"
#include <string.h>

// The 80-byte master font set containing characters 0 through F
const uint8_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8_init(Chip8* chip8){
    //clear the memory and registers
    memset(chip8,0,sizeof(Chip8));

    //make the program counter pointer pointes to the start of the game code
    chip8->pc = 0x200;

    chip8->I = 0;
    chip8->sp = 0;

    memcpy(&chip8->memory[0x050],chip8_fontset,sizeof(chip8_fontset));
    printf("[System]: Initialization complete. Built-in font set loaded to 0x050.\n");
}

int Chip8_ROM_LOADER(Chip8* chip8, const char* filename){
    FILE *file = fopen(filename, "rb");//read the file in binary mode
    if(file == NULL){
        printf("Error: Could not open file %s\n", filename);
        return 0;
    }
    fseek(file, 0, SEEK_END);//move the file pointer to the end of the file to determine the file size
    long file_size = ftell(file);//gives us the current position of the file pointer giving us the file size
    rewind(file);//move the file pointer back to the beginning of the file

    long max_size = MEMORY_SIZE - 0x200;//calculate the maximum size of the ROM that can be loaded into memory
    if(file_size > max_size){
        printf("Error: File size exceeds maximum allowed size of %ld bytes\n", max_size);
        fclose(file);
        return 0;
    }
    //start reading data in the virtual memory starting from 0x200
    size_t read_size = fread(&chip8->memory[0x200], sizeof(uint8_t),file_size,file); // read the file into memory starting at 0x200 on the chip8 struct
    if(read_size !=file_size){
        printf("Error: Could not read the entire file. Read %zu bytes out of %ld\n", read_size, file_size);
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;//successfully loaded the ROM into memory
}

void Chip8_cycle(Chip8* chip8){
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];//meshing the two bytes together to form the opcode
    chip8->pc += 2;

    uint16_t NNN = opcode & 0x0FFF; // Address (12 bits)
    uint8_t NN = opcode & 0x00FF; // 8-bit constant
    uint8_t N = opcode & 0x000f;// 4-bit constant
    uint8_t X = (opcode & 0x0F00) >> 8; // First register (4 bits)
    uint8_t Y = (opcode & 0x00F0) >> 4; // Second register (4 bits)
    
    switch(opcode & 0xF000){
        case 0x0000:
            // Sub-sorting because multiple opcodes start with 0x0000
            if (opcode == 0x00E0) {
                // 00E0: Clear screen display
                memset(chip8->display, 0, sizeof(chip8->display));
            } 
            else if (opcode == 0x00EE) {
                // 00EE: Return from a subroutine
                chip8->sp--;
                chip8->pc = chip8->stack[chip8->sp];
            }
            break;

        case 0x1000:
            // 1NNN: Jump to address NNN
            chip8->pc = NNN;
            break;

        case 0x2000:
            // 2NNN: Call subroutine at address NNN
            chip8->stack[chip8->sp] = chip8->pc; // Save current PC so we can come back
            chip8->sp++;
            chip8->pc = NNN;                     // Jump to subroutine address
            break;

        case 0x3000:
            // 3XNN: Skip next instruction if VX == NN
            if (chip8->V[X] == NN) {
                chip8->pc += 2;
            }
            break;

        case 0x6000:
            // 6XNN: Set register VX to value NN
            chip8->V[X] = NN;
            break;

        case 0x7000:
            // 7XNN: Add value NN to register VX (Carry flag is not changed!)
            chip8->V[X] += NN;
            break;
        
        default:
            printf("Unknown or unimplemented opcode: 0x%X\n", opcode);
            break;
    }

}