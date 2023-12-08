#ifndef CPU_HPP
#define CPU_HPP

#include "architecture.hpp"
#include "resstation.hpp"
#include "exec.hpp"

class Cpu {
public:
    void reset();
    void step();
    word getPc();

    word memory[1<<16];
    
private:
    void fetch();
    void decode();
    void dispatch();
    void execute();
    void writeback();

    word pc;
    word reg[16];
    uint8_t rat[16];
    Rob rob;
    JmpRs jmpRss[3];
    MovRs movRss[4];
    AguRs aguRss[4];
    AluRs aluRss[4];

    word fetched;
    Ctrl decoded;
    Cdb cdb;

    Exec jmpExec;
    Exec movExec;
    Exec aguExec;
    Exec aluExec;

    bool uopWasDispatched;
    bool uopIsValid;
};

#endif