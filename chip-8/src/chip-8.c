#include "chip-8.h"
#include <string.h>
#include<stdlib.h>


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
        case 0x0000:// display and flow control opcodes
            switch(opcode){
                case 0x00E0://clear the display
                    memset(chip8->display, 0,sizeof(chip8->display));
                    break;

                case 0x00EE://return from a subroutine
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    break;
                
                default://sys call - ignored in modern implementations 
                    break;
            }
        break;

        case 0x1000://jump to address NNN
            chip8->pc = NNN;
            break;
        
        case 0x2000://call subroutine at NNN
            chip8->stack[chip8->sp] = chip8->pc; //store the current program counter on the stack before jumping to the subroutine
            chip8->sp++; // move to the next stack position to store the next return address if needed
            chip8->pc = NNN; // jump to the subroutine at address NNN
            break;

        case 0x3000://skip the next instruction if VX equals NN
            if(chip8->V[X] == NN){
                chip8->pc += 2; // skip the next instruction by incrementing the program counter by 2 bytes
            }
            break;

        case 0x4000: //skip the next instruction if VX != NN
            if(chip8->V[X] != NN){
                chip8->pc += 2; // skip the next instruction by incrementing the program counter by 2 bytes
            }
            break;

        case 0x5000: //skip the next instruction if VX == VY
            if(chip8->V[X] == chip8->V[Y]){
                chip8->pc += 2; // skip the next instruction by incrementing the program counter by 2 bytes
            }
            break;

        case 0x6000: //set VX to NN
            chip8->V[X] = NN; // set the value of register VX to the constant NN
            break;

        case 0x7000: //add NN to VX (carry flag is not changed)
            chip8->V[X] += NN; // add the constant NN to the value of register VX and store the result back in VX
            break;
        
        case 0x8000: //arithmetic and Logic operations
            switch(N){
                case 0x0: //set VX to the value of VY
                    chip8->V[X] = chip8->V[Y];
                    break;
                case 0x1: //taking or
                    chip8->V[X] |= chip8->V[Y];
                    break;
                
                case 0x2: //taking and
                    chip8->V[X] &= chip8->V[Y];
                    break;
                
                case 0x3: //taking xor
                    chip8->V[X] ^= chip8->V[Y];
                    break;

                case 0x4: //add VY to VX and set VF to 1 if there's a carry, otherwise set it to 0
                    {
                        uint16_t sum = chip8->V[X] + chip8->V[Y]; // calculate the sum of VX and VY
                        chip8->V[0xF] = (sum > 0xFF) ? 1 : 0; // set VF to 1 if there's a carry (sum exceeds 255), otherwise set it to 0
                        chip8->V[X] = sum & 0xFF; // store the least significant byte of the sum back in VX
                    }
                    break;
                
                case 0x5: //subtract VY from VX and set VF to 0 if there's a borrow, otherwise set it to 1
                    {
                        chip8->V[0xF] = (chip8->V[X] >= chip8->V[Y]) ? 1 : 0; // set VF to 1 if there's no borrow (VX is greater than or equal to VY), otherwise set it to 0
                        chip8->V[X] = (chip8->V[X] - chip8->V[Y]) & 0xFF; // subtract VY from VX and store the result back in VX, ensuring it stays within 8 bits
                    }
                    break;

                case 0x6: // shift VX right by one. VF is set to the least significant bit of VX before the shift
                    chip8->V[0xF] = chip8->V[X] & 0x1; // set VF to the least significant bit of VX before the shift
                    chip8->V[X] >>= 1; // shift VX right by one
                    break;
                
                case 0x7: //subtract VX from VY and set VF to 0 if there's a borrow, otherwise set it to 1
                    {
                        chip8->V[0xF] = (chip8->V[Y] >= chip8->V[X]) ? 1 : 0; // set VF to 1 if there's no borrow (VY is greater than or equal to VX), otherwise set it to 0
                        chip8->V[X] = (chip8->V[Y] - chip8->V[X]) & 0xFF; // subtract VX from VY and store the result back in VX, ensuring it stays within 8 bits
                    }
                    break;
                
                case 0xE: // shift VX left by one. VF is set to the most significant bit of VX before the shift
                    chip8->V[0xF] = (chip8->V[X] & 0x80) >> 7; // set VF to the most significant bit of VX before the shift
                    chip8->V[X] <<= 1; // shift VX left by one
                    break;  

                default:
                    printf("Unknown or unimplemented opcode: 0x%X\n", opcode);
                    break;                
            }
            break;

        case 0x9000: // skip if VX != VY
            if(chip8->V[X] != chip8->V[Y]){
                chip8->pc += 2; // skip the next instruction by incrementing the program counter by 2 bytes
            }
            break;
        
        case 0xA000: // set I to the address NNN
            chip8->I = NNN;
            break;

        case 0xB000: // jump to the address NNN plus V0
            chip8->pc = NNN + chip8->V[0];
            break;
        
        case 0xC000: // set VX to a random byte AND NN
            chip8->V[X] = (rand() % 256) & NN; // generate a random byte (0-255) and perform a bitwise AND with NN, then store the result in VX
            break;

        case 0xD000://Draw the sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn't change after the execution of this instruction. VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn't happen
            {
                uint8_t x = chip8->V[X] % DISPLAY_WIDTH; // get the X coordinate from VX and wrap it around the display width
                uint8_t y = chip8->V[Y] % DISPLAY_HEIGHT; // get the Y coordinate from VY and wrap it around the display height
                chip8->V[0xF] = 0; // reset VF to 0 before drawing the sprite

                for(int row = 0; row < N; row++){
                    if(y + row >= DISPLAY_HEIGHT) break; // stop drawing if we go beyond the display height
                    uint8_t sprite_byte = chip8->memory[chip8->I + row]; // read each byte of the sprite from memory starting at address I
                    for(int col = 0; col < 8; col++){
                        if(x + col >= DISPLAY_WIDTH) break; // stop drawing if we go beyond the display width
                        uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 0x1; // get the current pixel from the sprite byte by shifting and masking
                        unsigned int screen_index = ((y+row)) * DISPLAY_WIDTH + ((x + col)); // calculate the index in the display array, wrapping around if necessary
                        if((sprite_pixel) == 1){ // check if the current bit in the sprite byte is set
                            if(chip8->display[screen_index] == 1){ // check if the pixel on the display is currently set
                                chip8->V[0xF] = 1; // set VF to 1 if a pixel is flipped from set to unset
                            }
                            chip8->display[screen_index] ^= 1; // flip the pixel on the display using XOR
                        }
                    }
                }
            }
            break;

        case 0xE000:// key input operations
            switch(NN){
                case 0x9E: // skip the next instruction if the key stored in VX is pressed
                    if (chip8->keypad[chip8->V[X]]) chip8->pc += 2;
                    break;
                
                case 0xA1: // skip the next instruction if the key stored in VX is not pressed
                    if (!chip8->keypad[chip8->V[X]]) chip8->pc += 2;
                    break;
            }
        break;

        case 0xF000: // miscellaneous and timer operations
            switch(NN){
                case 0x07: // set VX to the value of the delay timer
                    chip8->V[X] = chip8->delay_timer;
                    break;

                case 0x0A: // wait for a key press and store the result in VX
                {
                    int key_pressed = 0; // flag to indicate if a key has been pressed
                    for (int i=0;i<16;i++){
                        if(chip8->keypad[i]){ // check if any key is currently pressed
                            chip8->V[X] = i; // store the index of the pressed key in VX
                            key_pressed = 1; // set the flag to indicate a key has been pressed
                            break; // exit the loop once a key press is detected
                        }
                    }
                        //if no key is pressed go back to the previous instruction and check again in the next cycle
                        if(!key_pressed){
                            chip8->pc -= 2;
                        }
                }
                break;

                case 0x15: // set the delay timer to VX
                    chip8->delay_timer = chip8->V[X];
                    break;

                case 0x18: // set the sound timer to VX
                    chip8->sound_timer = chip8->V[X];
                    break;

                case 0x1E: // add VX to I. VF is not affected
                    chip8->I += chip8->V[X];
                    break;

                case 0x29: //set memory layout of the fornt character in VX.
                    chip8->I = 0x050 + chip8->V[X] * 5; // each character is 5 bytes long, so we multiply the character index by 5 to get the correct memory address
                    break;

                case 0x33: // binary coded decimal conversion.
                    chip8->memory[chip8->I] = chip8->V[X] / 100; // store the hundreds digit at address I
                    chip8->memory[chip8->I + 1] = (chip8->V[X] /10) % 10; // store the tens digit at address I + 1
                    chip8->memory[chip8->I + 2] = chip8->V[X] % 10; // store the ones digit at address I + 2
                    break;

                case 0x55: // store registers V0 through VX in memory starting at address I. I is set to I + X + 1 after operation
                {    
                for(int i =0; i<= X; i++){
                        chip8->memory[chip8->I + i] = chip8->V[i];// store the value of register Vi in memory starting at address I
                    }
                    chip8->I += X + 1; // increment I by X + 1 after storing the registers in memory
                }
                    break;
                
                
                case 0x65: // read registers V0 through VX from memory starting at address I. I is set to I + X + 1 after operation
                {    
                for(int i =0; i<= X; i++){
                        chip8->V[i] = chip8->memory[chip8->I + i]; // read the value from memory starting at address I and store it in register Vi
                    }
                    chip8->I += X + 1; // increment I by X + 1 after reading the registers from memory
                }
                    break;
            }
            break;
        
        default:
            printf("Unknown or unimplemented opcode: 0x%X\n", opcode);
            break;
    }

}