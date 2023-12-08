#ifndef EXEC_HPP
#define EXEC_HPP

#include "architecture.hpp"
#include "resstation.hpp"

class Exec {
public:
    Exec();
    Exec(int delay);
    void startInstr(ResStation*);
    void step();
    void reset();
    bool isBusy();
    bool valIsReady();
    Cdb getVal();
    ResStation* getInstr();
private:
    int state;
    int delay;
    ResStation *rs;
};

#endif