#include <iostream>
#include "cpu.hpp"

// This emulator is EXTREMELY INCOMPLETE
// Currently it can only do basic ALU operations and loads,
// but it does reorder them, kind of...

int main() {
    Cpu cpu;

    cpu.memory[0] = 0b0000000000000000; // ADD r0, r0, r0
    cpu.memory[1] = 0b0100000100001010; // LDI r1, 10
    cpu.memory[2] = 0b0100001000000001; // LDI r2, 1
    cpu.memory[3] = 0b0000001100010001; // ADD r3, r1, r1
    cpu.memory[4] = 0b0000001100010001; // ADD r3, r1, r1

    cpu.reset();

    for (int i = 0; i < 20; i++) cpu.step();

    return 0;
}