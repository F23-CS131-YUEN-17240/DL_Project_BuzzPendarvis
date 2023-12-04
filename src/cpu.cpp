#include "cpu.hpp"

#include <iostream>

void Cpu::reset() {
    pc = 0;
    reg[0] = 0;
    for (int i = 0; i < 16; i++) rat[i] = 0;
    rob.reset();
    for (int i = 0; i < 4; i++) aluRss[i].busy = false;
    for (int i = 0; i < 4; i++) aguRss[i].busy = false;
    for (int i = 0; i < 4; i++) movRss[i].busy = false;
    for (int i = 0; i < 3; i++) jmpRss[i].busy = false;
    cdb.tag = 0;
    dispatched = true;
}

void Cpu::step() {
    fetch();
    decode();
    dispatch();
    execute(); 
    writeback();
    fetch();
    decode();
    dispatch();
    writeback();
    fetch();
    decode();
    dispatch();
    writeback();
    fetch();
    decode();
    dispatch();
    writeback();
}

word Cpu::getPc() {
    return pc;
}

void Cpu::fetch() {
    if (!dispatched) return;
    fetched = memory[pc++]; 
    dispatched = false;
}
void Cpu::decode() {
    Ctrl uop;
    uop.address = pc;

    int category = (fetched & 0b11<<14) >> 14;
    switch (category) {
    case 0: 
        uop.immediate = 0;
        uop.resourceid = 3;
        uop.op = (fetched & 0b11<<12) >> 12;
        uop.s = fetched & 0xF;
        uop.t = (fetched & 0xF << 4) >> 4;
        uop.d = (fetched & 0xF << 8) >> 8;
        uop.write8b = false;
        uop.writeHi = false;
        uop.jumpNotBranch = false;
        break;
    case 1:
        uop.immediate = (fetched & 0xFF);
        uop.resourceid = 1;
        uop.op = (fetched & 0b11<<12) >> 12;
        uop.s = 0;
        uop.t = 0;
        uop.d = (fetched & 0xF << 8) >> 8;
        uop.write8b = (uop.op & 2);
        uop.writeHi = (uop.op & 1);
        uop.jumpNotBranch = false;

        if (uop.op == 0 && (uop.immediate & 0x80))
            uop.immediate |= 0xFF00;
        break;
    case 2: {
        int subcategory = (fetched & 0b11<<12) >> 12;
        switch (subcategory) {
        case 0:
            uop.immediate = (fetched & 0xF << 8) >> 8;
            uop.resourceid = 2;
            uop.op = 1;
            uop.s = fetched & 0xF;
            uop.t = (fetched & 0xF << 4) >> 4;
            uop.d = 0;
            uop.write8b = false;
            uop.writeHi = false;
            uop.jumpNotBranch = false;
            break;
        case 1:
            uop.immediate = 0;
            uop.resourceid = 2;
            uop.op = 0;
            uop.s = 0;
            uop.t = (fetched & 0xF << 4) >> 4;
            uop.d = (fetched & 0xF << 8) >> 8;
            uop.write8b = false;
            uop.writeHi = false;
            uop.jumpNotBranch = false;
            break;
        case 2:
            uop.immediate = 0;
            uop.resourceid = 0;
            uop.op = (fetched & 0x7 << 8) >> 8;
            uop.s = fetched & 0xF;
            uop.t = (fetched & 0xF << 4) >> 4;
            uop.d = 0;
            uop.write8b = false;
            uop.writeHi = false;
            uop.jumpNotBranch = false;
            break;
        case 3:
            break;
        }
        break; }
    case 3:
        uop.immediate = fetched & 0xFFF;
        uop.resourceid = 0;
        uop.op = (fetched & 0b11<<12) >> 12;
        uop.s = 0;
        uop.t = (uop.op & 2) ? (fetched & 0xF << 4) >> 4 : 0;
        uop.d = 0;
        uop.write8b = false;
        uop.writeHi = false;
        uop.jumpNotBranch = true;

        if (uop.op & 2) uop.immediate = 0;
        else uop.t = 0;
        break;
    }

    decoded = uop;
    dispatched = false;
}

void Cpu::dispatch() {
    if (rob.isFull()) return;
    if (dispatched) return;
    switch (decoded.resourceid) {
    case 0: // JMP
        for (int i = 0; i < 3; i++) {
            if (!aluRss[i].busy) {
                JmpRs rs;
                RobEntry robe;

                rs.busy = true;
                rs.op = decoded.op;
                rs.addr = decoded.address;
                rs.immediate = decoded.immediate;
                rs.qj = rat[decoded.s];
                rs.qk = rat[decoded.t];
                rs.vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
                rs.vj = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];

                rs.jumpNotBranch = decoded.jumpNotBranch;

                robe.dest = 0;
                robe.destIsAddr = false;
                robe.ready = false;
                robe.result = 0;

                rob.enqueue(robe);
                rs.tag = rob.tail;
                jmpRss[i] = rs;
                dispatched = true;
                break;
            }
        }
        break;
    case 1: // MOV
        for (int i = 0; i < 4; i++) {
            if (!aluRss[i].busy) {
                MovRs rs;
                RobEntry robe;

                rs.busy = true;
                rs.qj = 0;
                rs.qk = rat[decoded.t];
                rs.vj = decoded.immediate;
                rs.vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
                rs.write8b = decoded.write8b;
                rs.writeHi = decoded.writeHi;

                robe.dest = decoded.d;
                robe.destIsAddr = false;
                robe.ready = false;
                robe.result = 0;

                rob.enqueue(robe);
                rs.tag = rob.tail;
                movRss[i] = rs;
                
                if (decoded.d != 0) rat[decoded.d] = rs.tag;
                dispatched = true;
                break;
            }
        }
        break;
    case 2: // AGU
        for (int i = 0; i < 4; i++) {
            if (!aluRss[i].busy) {
                AguRs rs;
                RobEntry robe;

                rs.busy = true;
                rs.qj = rat[decoded.s];
                rs.qk = rat[decoded.t];
                rs.vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
                rs.vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];
                rs.write = bool(decoded.op);

                robe.dest = decoded.op ? 0 : decoded.d;
                robe.destIsAddr = bool(decoded.op);
                robe.ready = false;
                robe.result = 0;

                rob.enqueue(robe);
                rs.tag = rob.tail;
                aguRss[i] = rs;
                
                if (decoded.d != 0) rat[decoded.d] = rs.tag;
                dispatched = true;
                break;
            }
        }
        break;
    case 3: // ALU
        for (int i = 0; i < 4; i++) {
            if (!aluRss[i].busy) {
                AluRs rs;
                RobEntry robe;

                rs.busy = true;
                rs.op = decoded.op;
                rs.qj = rat[decoded.s];
                rs.qk = rat[decoded.t];
                rs.vj = rat[decoded.s] ? rat[decoded.s] : reg[decoded.s];
                rs.vk = rat[decoded.t] ? rat[decoded.t] : reg[decoded.t];

                robe.dest = decoded.d;
                robe.destIsAddr = false;
                robe.ready = false;
                robe.result = 0;

                rob.enqueue(robe);
                rs.tag = rob.tail;
                aluRss[i] = rs;

                if (decoded.d != 0) rat[decoded.d] = rs.tag;
                dispatched = true;
                break;
            }
        }
        break;
    }
}

void Cpu::execute() {

    if (cdb.tag != 0) for (int i = 0; i < 4; i++) {
        if (aluRss[i].qj == cdb.tag) {
            aluRss[i].qj = 0;
            aluRss[i].vj = cdb.value;
        }
        if (aluRss[i].qk == cdb.tag) {
            aluRss[i].qk = 0;
            aluRss[i].vk = cdb.value;
        }
        if (aguRss[i].qj == cdb.tag) {
            aguRss[i].qj = 0;
            aguRss[i].vj = cdb.value;
        }
        if (aguRss[i].qk == cdb.tag) {
            aguRss[i].qk = 0;
            aguRss[i].vk = cdb.value;
        }
        if (movRss[i].qj == cdb.tag) {
            aguRss[i].qj = 0;
            aguRss[i].vj = cdb.value;
        }
        if (movRss[i].qk == cdb.tag) {
            movRss[i].qk = 0;
            movRss[i].vk = cdb.value;
        }

        if (i > 2) continue;

        if (jmpRss[i].qj == cdb.tag) {
            jmpRss[i].qj = 0;
            jmpRss[i].vj = cdb.value;
        }
        if (jmpRss[i].qk == cdb.tag) {
            jmpRss[i].qk = 0;
            jmpRss[i].vk = cdb.value;
        }
    }

    int movix = -1;
    word movResult = 0;
    int aluix = -1;
    word aluResult = 0;

    for (int i = 0; i < 4; i++) {
        if (movRss[i].busy && movRss[i].qj == 0 && movRss[i].qk == 0) {
            if (movRss[i].write8b) {
                std::cout << "MOV NOT YET IMPLEMENTED" << std::endl;
                exit(-1);
            } else {
                std::cout << "LDI " << movRss[i].vj << std::endl;
                movResult = movRss[i].vj;
            }
            movix = i;
            break;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (aluRss[i].busy && aluRss[i].qj == 0 && aluRss[i].qk == 0) {
            switch (aluRss[i].op) {
            case 0:
                std::cout << "ADD " << aluRss[i].vj << " + " << aluRss[i].vk << std::endl;
                aluResult = aluRss[i].vj + aluRss[i].vk;
                break;
            case 1:
                std::cout << "SUB " << aluRss[i].vj << " - " << aluRss[i].vk << std::endl;
                aluResult = aluRss[i].vj - aluRss[i].vk;
                break;
            case 2:
                std::cout << "NAND " << aluRss[i].vj << " ~& " << aluRss[i].vk << std::endl;
                aluResult = ~(aluRss[i].vj & aluRss[i].vk);
                break;
            case 3:
                std::cout << "NOR " << aluRss[i].vj << " ~| " << aluRss[i].vk << std::endl;
                aluResult = ~(aluRss[i].vj | aluRss[i].vk);
                break;
            }
            aluix = i;
            break;
        }
    }

    int movTag = (movix >= 0) ? movRss[movix].tag : 0;
    int aluTag = (aluix >= 0) ? aluRss[aluix].tag : 0;
    movTag += (movTag < rob.head) * 16;
    aluTag += (aluTag < rob.head) * 16;

    if (aluTag < movTag) {
        cdb.tag = aluRss[aluix].tag;
        cdb.value = aluResult;
        aluRss[aluix].busy = false;
    } else if (aluTag > movTag) {
        cdb.tag = movRss[movix].tag;
        cdb.value = movResult;
        movRss[movix].busy = false;
    } else {
        cdb.tag = 0;
        cdb.value = 0;
    }
}
void Cpu::writeback() {
    // update ROB
    if (cdb.tag != 0) {
        //std::cout << "CDB " << cdb.value << " @" << int(cdb.tag) << std::endl;
        rob.rob[cdb.tag].result = cdb.value;
        rob.rob[cdb.tag].ready = true;
    }

    // commit
    if (!rob.isReady()) return;

    RobEntry robe = rob.dequeue();

    //std::cout << "Writing " << robe.result << " to r" << robe.dest << std::endl;

    if (robe.destIsAddr) memory[robe.dest] = robe.result;
    else {
        reg[robe.dest] = robe.result;
        rat[robe.dest] = 0;
    }
}

// ROB

void Cpu::Rob::reset() {
    head = 2;
    tail = 1;
    for (int i = 0; i < 16; i++) rob[i].ready = false;
    empty = true;
    full = false;
}
bool Cpu::Rob::isFull() {
    return full;
}
bool Cpu::Rob::isEmpty() {
    return empty;
}
bool Cpu::Rob::isReady() {
    return !isEmpty() && rob[head].ready;
}
void Cpu::Rob::enqueue(Cpu::RobEntry in) {
    tail = std::max(1, (tail + 1) & 0xF);
    rob[tail] = in;
    empty = false;
    if (std::max(1, (head + 1) & 0xF) == tail) full = true;
}
Cpu::RobEntry Cpu::Rob::dequeue() {
    RobEntry out = rob[head];
    head = std::max(1, (head + 1) & 0xF);
    full = false;
    if (std::max(1, (head + 1) & 0xF) == tail) empty = true;
    return out;
}