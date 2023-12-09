#include <iostream>
#include "cpu.hpp"

// This emulator is EXTREMELY INCOMPLETE
// Currently it can only do basic ALU operations and loads,
// but it does reorder them, kind of...

int main() {
    Cpu cpu;

    cpu.memory[0] = 0b0100000100000001; // LDI r1, 1
    cpu.memory[1] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[2] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[3] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[4] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[5] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[6] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[7] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[8] = 0b0000000100010001; // ADD r1, r1, r1
    cpu.memory[9] = 0b0000000100010001; // ADD r1, r1, r1

    cpu.reset();

    for (int i = 0; i < 40; i++) cpu.step();

    return 0;
}