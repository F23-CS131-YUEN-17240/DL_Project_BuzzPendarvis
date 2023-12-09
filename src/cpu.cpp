#include "cpu.hpp"

#include <iostream>

// helper functions, to make things prettier
void execOldest(Exec &ex, ResStation rs[], Rob rob);
void setIfOlder(Exec *&toSet, Exec &check, Rob &rob);

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
    jmpExec = Exec(4);
    movExec = Exec(1);
    aguExec = Exec(4);
    aluExec = Exec(5);
    uopIsValid = false;
    uopWasDispatched = true;
}

void Cpu::step() {
    writeback();
    execute();
    dispatch();
    
    fetch();
    decode();
}

word Cpu::getPc() {
    return pc;
}

void Cpu::fetch() {
    fetched = memory[pc];
}

void Cpu::decode() {
    if (!uopWasDispatched) return;

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

    uopIsValid = true;
    uopWasDispatched = false;
    decoded = uop;
}

void Cpu::dispatch() {
    if (rob.isFull() | !uopIsValid) return;

    switch (decoded.resourceid) {
    case 0: // JMP
        for (int i = 0; i < 3; i++) {
            if (!jmpRss[i].busy) {
                RobEntry robe;
                robe.dest = 0;
                robe.destIsAddr = false;
                robe.ready = false;
                robe.result = 0;
                rob.enqueue(robe);

                JmpRs rs = JmpRs(decoded, rob, reg, rat);
                jmpRss[i] = rs;

                pc++; // move on to next instruction
                uopIsValid = false;
                uopWasDispatched = true;
                break;
            }
        }
        break;
    case 1: // MOV
        for (int i = 0; i < 4; i++) {
            if (!movRss[i].busy) {
                RobEntry robe;
                robe.dest = decoded.d;
                robe.destIsAddr = false;
                robe.ready = false;
                robe.result = 0;
                rob.enqueue(robe);

                MovRs rs = MovRs(decoded, rob, reg, rat);
                movRss[i] = rs;
                
                if (decoded.d != 0) rat[decoded.d] = rs.tag;
                pc++; // move on to next instruction
                uopIsValid = false;
                uopWasDispatched = true;
                break;
            }
        }
        break;
    case 2: // AGU
        for (int i = 0; i < 4; i++) {
            if (!aguRss[i].busy) {
                RobEntry robe;
                robe.dest = decoded.op ? 0 : decoded.d;
                robe.destIsAddr = bool(decoded.op);
                robe.ready = false;
                robe.result = 0;
                rob.enqueue(robe);

                AguRs rs = AguRs(decoded, rob, reg, rat);
                rs.tag = rob.tail;
                aguRss[i] = rs;
                
                if (decoded.d != 0) rat[decoded.d] = rs.tag;
                pc++; // move on to next instruction
                uopIsValid = false;
                uopWasDispatched = true;
                break;
            }
        }
        break;
    case 3: // ALU
        for (int i = 0; i < 4; i++) {
            if (!aluRss[i].busy) {
                RobEntry robe;
                robe.dest = decoded.d;
                robe.destIsAddr = false;
                robe.ready = false;
                robe.result = 0;
                rob.enqueue(robe);

                AluRs rs = AluRs(decoded, rob, reg, rat);
                rs.tag = rob.tail;
                aluRss[i] = rs;

                if (decoded.d != 0) rat[decoded.d] = rs.tag;
                pc++; // move on to next instruction
                uopIsValid = false;
                uopWasDispatched = true;
                break;
            }
        }
        break;
    }
}

void Cpu::execute() {
    // reservation station listening step
    if (cdb.tag != 0) for (int i = 0; i < 4; i++) {
        if (i <= 2) jmpRss[i].update(cdb);
        movRss[i].update(cdb);
        aguRss[i].update(cdb);
        aluRss[i].update(cdb);
    }

    // steps should occur every clock cycle
    if (jmpExec.isBusy()) jmpExec.step();
    if (movExec.isBusy()) movExec.step();
    if (aguExec.isBusy()) aguExec.step();
    if (aluExec.isBusy()) aluExec.step();


    // prioritize older instructions by
    // searching for the oldest one by its
    // ROB tag. (ix = -1 default / none ready)
    
    // Begin executing oldest ready instructions.
    //execOldest(jmpExec, jmpRss, rob);
    execOldest(movExec, movRss, rob);
    //execOldest(aguExec, aguRss, rob);
    execOldest(aluExec, aluRss, rob);

    // Writeback the oldest available result
    Exec *toWb = nullptr;
    //setIfOlder(toWb, jmpExec, rob);
    setIfOlder(toWb, movExec, rob);
    //setIfOlder(toWb, aguExec, rob);
    setIfOlder(toWb, aluExec, rob);

    if (toWb != nullptr) {
        cdb = toWb->getVal();
        std::cout << "Finished: ";
        toWb->getInstr()->print();
        toWb->reset();
    } else {
        cdb.tag = 0;
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

    uint8_t dqFrom = rob.head;
    RobEntry robe = rob.dequeue();

    std::cout << "Writing " << robe.result << " to r" << robe.dest << std::endl;

    if (robe.destIsAddr) memory[robe.dest] = robe.result;
    else {
        reg[robe.dest] = robe.result;
        if (rat[robe.dest] == dqFrom) rat[robe.dest] = 0;
    }
}

void execOldest(Exec &ex, ResStation rs[], Rob rob) {
    if (!ex.isBusy()){
        int ix = -1; // keeps track of current oldest found
        for (int i = 0; i < 4; i++) {
            if (!rs[i].ready()) continue; // skip if not ready

            if (ix == -1) ix = i;
            else {
                uint8_t t1 = rob.tagc(rs[ix].tag);
                uint8_t t2 = rob.tagc(rs[i].tag);
                ix = (t1 < t2) ? ix : i;
            }
        }
        if (ix != -1) ex.startInstr(&rs[ix]);
    }
}

void setIfOlder(Exec *&toSet, Exec &check, Rob &rob) {
    if (check.valIsReady()) {
        if (
            toSet == nullptr
            || (rob.tagc(toSet->getVal().tag) < rob.tagc(check.getVal().tag))
        ) {
            toSet = &check;
        }
    }
} 