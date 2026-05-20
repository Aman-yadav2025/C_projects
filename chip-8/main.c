#include "chip-8.h"
#include <SDL2/SDL.h>

#define WINDOW_SCALE 15

// Keypad Mapping: Maps standard QWERTY keys to the CHIP-8 hex keypad
// 1 2 3 C -> 1 2 3 4
// 4 5 6 D -> q w e r
// 7 8 9 E -> a s d f
// A 0 B F -> z x c v
uint8_t keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

int main(int argc, char** argv){
    if(argc != 2){
        printf("Usage: %s <ROM file>\n", argv[0]);
        return 1;
    }
    //intialize the chip8 system and load the game into memory
    Chip8 chip8;
    Chip8_init(&chip8);
    if(!(Chip8_ROM_LOADER(&chip8, argv[1]))){
        return 1;
    }

    if(SDL_Init(SDL_INIT_VIDEO) <0 ){
        printf("Error: Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH * WINDOW_SCALE, DISPLAY_HEIGHT * WINDOW_SCALE, SDL_WINDOW_SHOWN);
    
    if(window == NULL){//fail safe
        printf("Error: Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    //create a renderer for the window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL){//fail safe
        printf("Error: Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //create a texture to represent the CHIP-8 display
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if(texture == NULL){//fail safe
        printf("Error: Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    uint32_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT]; // buffer to hold pixel data for the texture

    bool running = true;
    SDL_Event event;

    //main loop
    while(running){
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running  = false;
            }
            else if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
                bool is_pressed = (event.type == SDL_KEYDOWN);
                for(int i = 0; i<16;i++){
                    if(event.key.keysym.sym == keymap[i]){
                        chip8.keypad[i] = is_pressed ? 1 : 0; // update the state of the corresponding key in the CHIP-8 keypad array
                        break;
                    }
                }
            }
        }
        //exexute cpu cycles
        //runes at 500-700 Hz at 60Hz we can do 10 CPU cycles per frame
        for(int i =0; i<10; i++){
            Chip8_cycle(&chip8);
        }
        //update Timer 
        if(chip8.delay_timer > 0){
            chip8.delay_timer--;
        }
        if(chip8.sound_timer > 0){
            if(chip8.sound_timer == 1){
                printf("BEEP!\n");
            }
            chip8.sound_timer--;
        }
        //Render the display
        for(int i=0 ; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++){
            pixels[i] = chip8.display[i] ? 0xFFFFFFFF : 0xFF000000; // set pixel to white if it's on, black if it's off
        }
        SDL_UpdateTexture(texture, NULL, pixels, DISPLAY_WIDTH * sizeof(uint32_t)); // update the texture with the new pixel data
        SDL_RenderClear(renderer); // clear the renderer
        SDL_RenderCopy(renderer, texture, NULL,NULL); // copy the texture to the renderer
        SDL_RenderPresent(renderer); // present the updated renderer to the window

        SDL_Delay(16); // delay to cap the frame rate at ~60 FPS
    }


    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}