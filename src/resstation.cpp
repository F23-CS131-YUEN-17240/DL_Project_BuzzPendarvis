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
    std::cout << busy << std::endl;
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
JmpRs::JmpRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]) {
    busy = true;
    op = decoded.op;
    addr = decoded.address;
    immediate = decoded.immediate;
    qj = rat[decoded.s];
    qk = rat[decoded.t];
    vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
    vj = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    jumpNotBranch = decoded.jumpNotBranch;
    this->tag = tag;
}
MovRs::MovRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]) {
    busy = true;
    qj = 0;
    qk = rat[decoded.t];
    vj = decoded.immediate;
    vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    write8b = decoded.write8b;
    writeHi = decoded.writeHi;
    this->tag = tag;
}
AguRs::AguRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]) {
    busy = true;
    qj = rat[decoded.s];
    qk = rat[decoded.t];
    vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
    vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    write = bool(decoded.op);
    this->tag = tag;
}
AluRs::AluRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]) {
    busy = true;
    op = decoded.op;
    qj = rat[decoded.s];
    qk = rat[decoded.t];
    vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
    vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
    this->tag = tag;
}



// Execution
word JmpRs::execute() {
    std::cerr << "ATTEMPTED TO EXECUTE NON-IMPLEMENTED INSTR" << std::endl;
    return 0;
}
word MovRs::execute() {
    if (write8b) {
        std::cout << "MOV NOT YET IMPLEMENTED" << std::endl;
        exit(-1);
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
    std::cout << "ALU ";
    ResStation::print();
}