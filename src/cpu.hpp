#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>

typedef uint16_t word;

// Execution units:
//   ALU - for math ops
//   MOV - just moves data
//   JMP - jump address calculator

class Cpu {
public:
    void reset();
    void step();
    word getPc();

    word memory[1<<16];
private:
    struct Ctrl {
        word address;
        word immediate;
        uint8_t resourceid; // JMP 0 MOV 1 AGU 2 ALU 3
        uint8_t op;
        uint8_t s, t, d;
        bool write8b;
        bool writeHi;
        bool jumpNotBranch;
    };
    struct Cdb {
        word value;
        uint8_t tag;
    };
    struct RobEntry {
        word dest;
        word result;
        bool destIsAddr;
        bool ready;
    };
    class Rob {
    public:
        void reset();
        bool isFull();
        bool isEmpty();
        bool isReady();
        void enqueue(RobEntry);
        RobEntry dequeue();

        RobEntry rob[16];
        uint8_t head;
        uint8_t tail;
        bool empty;
        bool full;
    };
    struct JmpRs {
        word addr;
        word immediate;
        word vj, vk;
        uint8_t qj, qk;
        uint8_t op;
        uint8_t tag;
        bool jumpNotBranch;
        bool busy;
    };
    struct MovRs {
        word vj, vk;
        uint8_t qj, qk;
        uint8_t tag;
        bool write8b;
        bool writeHi;
        bool busy;
    };
    struct AguRs {
        word vj, vk;
        uint8_t qj, qk;
        uint8_t tag;
        bool write;
        bool busy;
    };
    struct AluRs {
        word vj, vk;
        uint8_t qj, qk;
        uint8_t op;
        uint8_t tag;
        bool busy;
    };

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

    bool dispatched;
};

#endif