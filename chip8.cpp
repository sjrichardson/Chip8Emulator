#include "chip8.h"
#include <fstream>

chip8::chip8() {

};

void chip8::emulateCycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
};


bool chip8::loadApplication(char const* fileName) {
    // open file stream
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        // Get file size and set buffer accordlingly
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        // Fill buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // Load buffer into memory (beginning at 0x200 to match standard)
        // To figure out: why do we place first value at 201 instead of 200
        for (long i = 0; i < size; ++i) {
            memory[START_ADDRESS + i] = buffer[i];
        }
        
        // Free buffer
        delete[] buffer;

    } 
};

chip8::~chip8() {

};

void chip8::init() {
    pc = START_ADDRESS; // program counter begins at 0x200

    // Reset current opcode, index register, and stack pointer
    opcode = 0;
    index = 0;
    sp = 0;

    // Clear display
    // Clear stack
    // Clear registers V0-VF
    // Clear memory

    // Load fontset
    for (int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = chip8_fontset[i];
    }

    // Reset timers
}

