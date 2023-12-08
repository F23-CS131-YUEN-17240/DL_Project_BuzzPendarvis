#ifndef RESSTATION_HPP
#define RESSTATION_HPP

#include "architecture.hpp"

class ResStation {
public:
    word addr;
    word vj, vk;
    uint8_t qj, qk;
    uint8_t tag;
    bool busy;

    virtual word execute();
    bool ready();
    void update(Cdb);
    virtual void print();
};

class JmpRs : public ResStation {
public:
    JmpRs() {}
    JmpRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]);
    word immediate;
    uint8_t op;
    bool jumpNotBranch;

    virtual word execute();
    virtual void print();
};

class MovRs : public ResStation {
public:
    MovRs() {}
    MovRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]);
    bool write8b;
    bool writeHi;

    virtual word execute();
    virtual void print();
};

class AguRs : public ResStation {
public:
    AguRs() {}
    AguRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]);
    bool write;

    virtual word execute();
    virtual void print();
};

class AluRs : public ResStation {
public:
    AluRs() {}
    AluRs(Ctrl decoded, uint8_t tag, word reg[], uint8_t rat[]);
    uint8_t op;

    virtual word execute();
    virtual void print();
};

#endif