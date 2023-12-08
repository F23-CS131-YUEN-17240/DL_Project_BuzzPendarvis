#include <iostream>
#include "cpu.hpp"

// This emulator is EXTREMELY INCOMPLETE
// Currently it can only do basic ALU operations and loads,
// but it does reorder them, kind of...

int main() {
    Cpu cpu;

    cpu.memory[0] = 0b0100001000001001; // LDI r2, 9
    cpu.memory[1] = 0b0100001100001010; // LDI r3, 10
    cpu.memory[2] = 0b0000000100110010; // ADD r1, r2, r3
    cpu.memory[3] = 0b0001000100110001; // SUB r1, r1, r3
    cpu.memory[4] = 0b0000101000000000; // ADD r10, r0, r0
    //cpu.memory[5] = 0b0100000100000000; // LDI r1, 0 breaks?
    cpu.memory[5] = 0b0100010100000000; // LDI r5, 0

    cpu.memory[6] = 0b0010011110011000; // NAND r7, r8, r9
    cpu.memory[7] = 0b0011101011001011; // NOR r10, r11, r12

    cpu.reset();

    for (int i = 0; i < 20; i++) cpu.step();

    return 0;
}