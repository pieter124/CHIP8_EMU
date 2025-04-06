#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include "chip8.hpp"


Chip8::Chip8() {
    // Reset and clear values 
    PC = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    draw_flag = false;

    for (int i = 0; i < 16; i++) {
        V[i] = 0;
        stack[i] = 0;
        keypad[i] = 0;
    }

    for (int i = 0; i < 4096; i++) {
        RAM[i] = 0;
    }

    for (int i = 0; i < 2048; i++) {
        graphics[i] = 0;
    }

    // Load fontset
    unsigned char chip8_fontset[80] = {
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

    for(int i = 0; i < 80; i++) {
        RAM[i] = chip8_fontset[i];
    }

    // Reset timers
    sound_timer = 0;
    delay_timer = 0;

}

bool Chip8::loadRom(char* string) {
    std::ifstream fp(string,  std::ifstream::binary | std::ifstream::in);
    if (!fp.is_open()) {
        return false;
    }
    char buffer;
    int i = 0;
    while (fp.good()) {
        if (i + 512 >= 4096) {
            return false;
        }
        fp.get(buffer);
        RAM[512 + i] = buffer;
        i += 1;
    }
    fp.close();
    return true;
}

void Chip8::emulateCycle() {
    // Fetch opcode (bit mangling)
    opcode = (RAM[PC] << 8) | (RAM[PC + 1]);
    int x;
    int y;
    int height;
    bool key_pressed;
    
    // Decode opcode
    switch(opcode & 0xF000) {
        // Opcodes
        case 0x0000:
            switch(opcode) {
                case 0x00E0: // Clear screen
                    for (int i = 0; i < 2048; i++) {
                        graphics[i] = 0;
                    }
                    draw_flag = true;
                    PC += 2;
                    break;

                case 0x00EE: // Return from subroutine
                    sp--;
                    PC = stack[sp];
                    PC += 2;
                    break;

                default:
                    std::cerr << "Invalid opcode: 0x" << opcode << std::endl;
                    break;
            }
            break;

        case 0x1000: // Set PC to NNN
            PC = opcode & 0x0FFF;
            break;

        case 0x2000: // Calls subroutine at NNN
            stack[sp] = PC;
            sp++;
            PC = opcode & 0x0FFF;
            break;

        case 0x3000: // Skips instruction if V[x] is 0x00NN
            x = (opcode & 0x0F00) >> 8;
            PC += 2;
            if (V[x] == (opcode & 0x00FF)) {
                PC += 2;
            }
            break;

        case 0x4000: // Skips instruction if V[x] is not 0x00NN
            x = (opcode & 0x0F00) >> 8;
            PC += 2;
            if (V[x] != (opcode & 0x00FF)) {
                PC += 2;
            }
            break;

        case 0x5000: // Skips instruction if V[x] is V[y]
            x = (opcode & 0x0F00) >> 8;
            y = (opcode & 0x00F0) >> 4;
            PC += 2;
            if (V[x] == V[y]) {
                PC += 2;
            }
            break;
        
        case 0x6000: // Sets V[x] to 0x00NN
            x = (opcode & 0x0F00) >> 8;
            PC += 2;
            V[x] = (opcode & 0x00FF);
            break;
        
        case 0x7000: // Adds 0x00NN to V[x]
            x = (opcode & 0x0F00) >> 8;
            PC += 2;
            V[x] += (opcode & 0x00FF);
            V[x] = (uint8_t) V[x];
            break;

        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: // VX = VY
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    V[x] = V[y];
                    PC += 2;
                    break; 
                
                case 0x0001: // VX = VX | VY
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    V[x] |= V[y];
                    PC += 2;
                    break;

                case 0x0002: // VX = VX & VY
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    V[x] &= V[y];
                    V[0xF] = 0;
                    PC += 2;
                    break;

                case 0x0003: // VX = VX ^ VY
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    V[x] ^= V[y];
                    V[0xF] = 0;
                    PC += 2;
                    break;
                
                case 0x0004: // VX = VX + VY
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    if (V[x] + V[y] > 0xFF) {
                        V[0xF] = 1;
                    }
                    else {
                        V[0xF] = 0;
                    }
                    V[x] += V[y];
                    V[x] = (uint8_t) V[x];
                    PC += 2;
                    break;
                
                case 0x0005: // VX = VX - VY
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    if (V[x] < V[y]) {
                        V[0xF] = 0;
                    }
                    else {
                        V[0xF] = 1;
                    }
                    V[x] = (uint8_t) V[x] - (uint8_t) V[y];
                    PC += 2;
                    break;
            
                case 0x0006: // VX = VX >> 1
                    x = (opcode & 0x0F00) >> 8;
                    V[0xF] = V[x] & 0x1;
                    V[x] >>= 1;
                    V[x] = (uint8_t) V[x];
                    PC += 2;
                    break;
                
                case 0x0007: // VX = VY - VX
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    if (V[x] > V[y]) {
                        V[0xF] = 0;
                    }
                    else {
                        V[0xF] = 1;
                    }
                    V[x] = (uint8_t) V[y] - (uint8_t) V[x];
                    PC += 2;
                    break;
                
                case 0x000E: // VX = VX << 1
                    x = (opcode & 0x0F00) >> 8;
                    V[0xF] = V[x] >> 7;
                    V[0xF] = (uint8_t) V[0xF];
                    V[x] <<= 1;
                    V[x] = (uint8_t) V[x];
                    PC += 2;
                    break;
                
                default:
                    std::cerr << "Invalid opcode: 0x" <<  opcode << std::endl;
                    break;
            }
            break;    

        case 0x9000: // Skips instruction if V[x] is not V[y]
            x = (opcode & 0x0F00) >> 8;
            y = (opcode & 0x00F0) >> 4;     
            PC += 2;
            if (V[x] != V[y]) {
                PC += 2;
            }
            break;

        case 0xA000: // Sets I to the address NNN
            I = opcode & 0x0FFF;
            PC += 2;
            break;
        
        case 0xB000: // Jumps to address 0x0NNN plus V[0]
            PC = (opcode & 0x0FFF) + V[0];
            break;

        case 0xC000: // Random Num Gen
            x = (opcode & 0x0F00) >> 8;
            V[x] = (rand() % 256) & (opcode & 0x00FF);
            PC += 2;
            break;

        case 0xD000: // Draw/Display
            x = (opcode & 0x0F00) >> 8;
            y = (opcode & 0x00F0) >> 4;
            height = opcode & 0x000F;
            unsigned short pixel;
            V[0xF] = 0;
            for (int i = 0; i < height; i++) {
                int pixel = RAM[I + i];
                for (int j = 0; j < 8; j++) {
                    if ((pixel & (0x80 >> j)) != 0) {
                        int index = ( (V[x] + j) + ((V[y] + i) * 64)) % 2048;
                        if (graphics[index] == 1)   {
                            V[0xF] = 1;
                        }
                        graphics[index] ^= 1;
                    }
                }
            }
            draw_flag = true;
            PC += 2;
            break;
            
        case 0xE000:
            switch(opcode & 0x00FF) {
                case 0x009E: // Skips instruction if key corresponding to value in VX is pressed
                    x = (opcode & 0x0F00) >> 8;
                    PC += 2;
                    if(keypad[V[x]] != 0) {
                        PC += 2;
                    }
                    break;
                case 0x00A1: // Skips instruction if key corresponding to value in VX is not pressed
                    x = (opcode & 0x0F00) >> 8;
                    PC += 2;
                    if(keypad[V[x]] == 0) {
                        PC += 2;
                    }
                    break;
                default:
                    std::cerr << "Invalid opcode: 0x" << opcode << std::endl;
                    break;
            }
            break;

        case 0xF000:
            switch(opcode & 0x00FF) {
                case 0x0007: // Sets VX to the current value of the delay timer
                    x = (opcode & 0x0F00) >> 8;
                    V[x] = delay_timer;
                    PC += 2;
                    break;

                case 0x000A: // Wait for a key press and store in VX
                    key_pressed = false;
                    x = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i < 16; i++) {
                        if (keypad[i] != 0) {
                            key_pressed = true;
                            V[x] = (uint8_t) i;
                        }
                    }
                    if (key_pressed) {
                        PC += 2;
                    }
                    break;

                case 0x0015: // Sets the delay timer to the value in VX
                    x = (opcode & 0x0F00) >> 8;
                    delay_timer = V[x];
                    PC += 2;
                    break;
                
                case 0x0018: // Sets the sound timer to the value in VX
                    x = (opcode & 0x0F00) >> 8;
                    sound_timer = V[x];
                    PC += 2;
                    break;

                case 0x001E:
                    x = (opcode & 0x0F00) >> 8;
                    if (I + V[x] > 0xFFF) {
                        V[0xF] = 1;
                    }
                    else {
                        V[0xF] = 0;
                    }
                    I += V[x];
                    I = (uint16_t) I;
                    PC += 2;
                    break;

                case 0x0029:
                    x = (opcode & 0x0F00) >> 8;
                    I = V[x] * 0x5;
                    PC += 2;
                    break;

                case 0x0033:
                    x = (opcode & 0x0F00) >> 8;

                    RAM[I] = (uint8_t) V[x] / 100;
                    RAM[I] = (uint8_t) RAM[I];

                    RAM[I + 1] = (uint8_t) (V[x] / 10) % 10;
                    RAM[I + 1] = (uint8_t) RAM[I + 1];

                    RAM[I + 2] = (uint8_t) (V[x] % 100) % 10;
                    RAM[I + 2] = (uint8_t) RAM[I + 2];

                    PC += 2;
                    break;

                case 0x0055:
                    x = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i <= x; i++) {
                        RAM[I + i] = V[i];
                    }
                    I = I + x + 1;
                    I = (uint16_t) I;
                    PC += 2;
                    break;

                case 0x0065:
                    x = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i <= x; i++) {
                        V[i] = RAM[I + i];
                    }
                    I = I + x + 1;
                    I = (uint16_t) I;
                    PC += 2;
                    break;

                default:
                    std::cerr << "Invalid opcode: 0x" << opcode << std::endl;
                    break;
            }
            break;

        default:
            std::cerr << "Invalid opcode: 0x" << opcode << std::endl;
            break;
    }

    // Update timers
    if (delay_timer > 0) {
        --delay_timer;
    }
    if (sound_timer > 0) {
        --sound_timer;
    }
}

void Chip8::set_keypad_value(int index, int val) {
    keypad[index] = val;
}

bool Chip8::get_draw_flag() {
    return draw_flag;
}

void Chip8::set_draw_flag(bool cond) {
    draw_flag = cond;
}

int Chip8::get_graphic_value(int index) {
    return graphics[index];
}

Chip8::~Chip8() {
}


