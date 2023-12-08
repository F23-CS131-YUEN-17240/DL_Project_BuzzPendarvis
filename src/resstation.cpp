#include "resstation.hpp"
#include <iostream>

word ResStation::execute() {
    std::cerr << "ATTEMPTED TO EXECUTE NON-IMPLEMENTED INSTR" << std::endl;
    return 0;
}
void ResStation::print() {
    std::cout << "@"<< int(tag) << " ";
    if (qj == 0) std::cout << vj << " ";
    else std::cout << " (" << int(qj) << ") ";
    if (qk == 0) std::cout << vk << " ";
    else std::cout << " (" << int(qk) << ") ";
    std::cout << std::endl;
}
bool ResStation::ready() {
    return (busy && qj == 0 && qk == 0);
}
void ResStation::update(Cdb cdb) {
    if (qj == cdb.tag) {
        qj = 0;
        vj = cdb.value;
    }
    if (qk == cdb.tag) {
        qk = 0;
        vk = cdb.value;
    }
}

// Constructors
JmpRs::JmpRs(Ctrl decoded, Rob &rob, word reg[], uint8_t rat[]) {
    busy = true;
    op = decoded.op;
    addr = decoded.address;
    immediate = decoded.immediate;
    qj = rat[decoded.s];
    qk = rat[decoded.t];
    vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
    vj = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    jumpNotBranch = decoded.jumpNotBranch;
    this->tag = rob.tail;
}
MovRs::MovRs(Ctrl decoded, Rob &rob, word reg[], uint8_t rat[]) {
    busy = true;
    qj = 0;
    qk = rat[decoded.t];
    vj = decoded.immediate;
    vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    write8b = decoded.write8b;
    writeHi = decoded.writeHi;
    this->tag = rob.tail;
}
AguRs::AguRs(Ctrl decoded, Rob &rob, word reg[], uint8_t rat[]) {
    busy = true;
    qj = rat[decoded.s];
    qk = rat[decoded.t];
    vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
    vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    write = bool(decoded.op);
    this->tag = rob.tail;
}
AluRs::AluRs(Ctrl decoded, Rob &rob, word reg[], uint8_t rat[]) {
    busy = true;
    op = decoded.op;
    if (rob.rob[rat[decoded.s]].ready) {
        qj = 0;
        vj = rob.rob[rat[decoded.s]].result;
    } else {
        qj = rat[decoded.s];
        vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
    }
    if (rob.rob[rat[decoded.t]].ready) {
        qk = 0;
        vk = rob.rob[rat[decoded.t]].result;
    } else {
        qk = rat[decoded.t];
        vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    }
    this->tag = rob.tail;
}



// Execution
word JmpRs::execute() {
    std::cerr << "ATTEMPTED TO EXECUTE NON-IMPLEMENTED INSTR" << std::endl;
    return 0;
}
word MovRs::execute() {
    if (write8b) {
        return writeHi ? (vk & 0x0F) | vj : (vk & 0xF) | (vj << 8);
    } else {
        return vj;
    }
}
word AguRs::execute() {
    std::cerr << "ATTEMPTED TO EXECUTE NON-IMPLEMENTED INSTR" << std::endl;
    return 0;
}
word AluRs::execute() {
    word result;
    switch (op) {
    case 0:
        return vj + vk;
        break;
    case 1:
        return vj - vk;
        break;
    case 2:
        return ~(vj & vk);
        break;
    case 3:
        return ~(vj | vk);
        break;
    default:
        std::cerr << "INTERNAL ERROR: ALU EXECUTE" << std::endl;
        exit(-1);
    }
}



// display
void JmpRs::print() {
    std::cout << "JMP ";
    ResStation::print();
}
void MovRs::print() {
    std::cout << "MOV ";
    ResStation::print();
}
void AguRs::print() {
    std::cout << "AGU ";
    ResStation::print();
}
void AluRs::print() {
    switch (op) {
    case 0:
        std::cout << "ADD ";
        break;
    case 1:
        std::cout << "SUB ";
        break;
    case 2:
        std::cout << "NAND ";
        break;
    case 3:
        std::cout << "NOR ";
        break;
    }
    ResStation::print();
}