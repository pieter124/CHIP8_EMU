#include <iostream>
#include <zconf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_events.h>
#include "chip8.hpp"

uint8_t keymap[16] = {
    SDLK_1, SDLK_2, SDLK_3, SDLK_4,    
    SDLK_q, SDLK_w, SDLK_e, SDLK_r, 
    SDLK_a, SDLK_s, SDLK_d, SDLK_f,  
    SDLK_z, SDLK_x, SDLK_c, SDLK_v  
};


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage :" << *argv << " Path to ROM" << std::endl;
        exit(1);
    }

    Chip8 emu;
    if (!emu.loadRom(*(argv + 1))) {
        std::cerr << "ROM could not be loaded." << std::endl;
        exit(1);
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    const int width = 640;
    const int height = 320;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Error initialising SDL " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    window = SDL_CreateWindow("CHIP-8_EMU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Error creating window SDL " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    } 

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        std::cerr << "Error creating renderer SDL " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    SDL_RenderSetLogicalSize(renderer, width, height);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == NULL) {
        std::cerr << "Error creating texture SDL " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }
    while (true) {
        emu.emulateCycle();
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(0);
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    exit(0);
                }
                for (int i = 0; i < 16; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        emu.set_keypad_value(i, 1);
                    }
                }
            }
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        emu.set_keypad_value(i, 0);
                    }
                }
            }
        }
        if (emu.get_draw_flag()) 
        {
            emu.set_draw_flag(false);
            uint32_t pixels[2048];
            for (int i = 0; i < 2048; i++) {
                if (emu.get_graphic_value(i) == 0) {
                    pixels[i] = 0xFF000000;
                }
                else {
                    pixels[i] = 0xFFFFFFFF;
                }
            }
        SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        }
        usleep(1500);
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}