#include <cstdint>
#include <iostream>

class Chip8 {
    public:
        Chip8();
        bool loadRom(char*);
        void emulateCycle();
        void set_keypad_value(int, int);
        bool get_draw_flag();
        void set_draw_flag(bool);
        int get_graphic_value(int);
        ~Chip8();        

    private:
        int graphics[2048];    
        uint8_t RAM[4096];
        uint8_t V[16];
        unsigned short stack[16];
        int keypad[16];
        unsigned short opcode;
        unsigned short PC;
        unsigned short sp;
        unsigned short I;
        unsigned char delay_timer;
        unsigned char sound_timer;
        bool draw_flag;
};