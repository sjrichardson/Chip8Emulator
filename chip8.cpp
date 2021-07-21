#include "chip8.h"
#include <fstream>

chip8::chip8():randGen(std::chrono::system_clock::now().time_since_epoch().count()) {

    // Init RNG
    randByte = std::uniform_int_distribution<uint8_t>(0,255U);
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

// Clear display
void chip8::OP_00E0() {
    memset(video, 0, sizeof(video));
}

// Return from a subroutine
void chip8::OP_00EE() {
    --sp;
    pc = stack[sp];
}

// Jump to location nnn
void chip8::OP_1nnn() {
    uint16_t address = opcode & 0x0FFFu;

    pc = address;
}

// Call subroutine at nnn
void chip8::OP_2nnn() {
    uint16_t address = opcode & 0x0FFFu;
    
    stack[sp++] = pc;
    pc = address;
}

// Skip next instruction if Vx = kk
void chip8::OP_3xkk() {
    uint8_t Vx =  (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte) {
        pc += 2;
    }
}

// Skip next instruction if Vx != kk
void chip8::OP_4xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] != byte) {
        pc += 2;
    }
}

// Skip next instruction if Vx = Vy
void chip8::OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy]) {
        pc += 2;
    }
}

// Set Vx = kk
void chip8::OP_6xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
}

// Set Vx = Vx + kk
void chip8::OP_7xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] += byte;
}

// Set Vx = Vy
void chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

// Set Vx = Vx OR Vy
void chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
}

// Set Vx = Vx AND Vy
void chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
}

// Set Vx = Vx XOR Vy
void chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
}

// Set Vx = Vx + Vy, set VF = carry
void chip8::OP_8xy4() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];
    
    // if sum > 255 set carry bit to 1
    registers[0xF] = (sum > 0xFFu) 1 : 0;
    // convert sum to uint8_t
    registers[Vx] = sum & 0xFFu;
}

// Set Vx = Vx - Vy, set VF = NOT borrow
void chip8::OP_8xy5() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    
    // if Vx > Vy set carry bit to 1
    registers[0xF] = (registers[Vx] > registers[Vy]) ? 1 : 0;
    registers[Vx] -= registers[Vy];
}

// Set Vx = Vx SHR 1
void chip8::OP_8xy6() {
    uint8_t Vx = (opcode & 0x0F00) >> 8u;

    // Set LSB in VF
    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] >> 1;
}

// Set Vx = Vy - Vx, set VF = NOT borrow
void chip8::OP_8xy7() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    // if Vy > Vx set carry bit to 1
    registers[0xF] = (registers[Vy] > registers[Vx]) ? 1 : 0;
    registers[Vx] = registers[Vy] - registers[Vx];
}

// Set Vx = Vx SHL 1
void chip8::OP_8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Save MSB in VF
    //0x80 (128) is the most significant bit in a byte
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
    registers[Vx] <<= 1;
}