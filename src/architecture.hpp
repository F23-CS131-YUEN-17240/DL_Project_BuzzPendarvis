#ifndef ARCHITECTURE_HPP
#define ARCHITECTURE_HPP

#include <cstdint>

typedef uint16_t word;

/// @brief common data bus data/tag
struct Cdb {
    word value;
    uint8_t tag;
};

/// @brief micro-ops
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

/// @brief ReOrder Buffer Entry
struct RobEntry {
    word dest;
    word result;
    bool destIsAddr;
    bool ready;
};

/// @brief ReOrder Buffer
class Rob {
public:
    void reset();
    bool isFull();
    bool isEmpty();
    bool isReady();
    void enqueue(RobEntry);
    RobEntry dequeue();
    /// @brief convert tag to relative absolute position.
    uint8_t tagc(uint8_t tag);

    RobEntry rob[16];
    uint8_t head;
    uint8_t tail;
    bool empty;
    bool full;
};

#endif