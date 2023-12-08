#include "exec.hpp"
#include <iostream>

Exec::Exec() {
    this->rs = nullptr;
}
Exec::Exec(int delay) {
    this->state = 0;
    this->delay = delay;
    this->rs = nullptr;
}
void Exec::startInstr(ResStation* rs) {
    this->rs = rs;
    step();
}
void Exec::step() {
    state = (state+1 > delay) ? delay : state+1;
}
void Exec::reset() {
    rs->busy = false;
    state = 0;
    rs = nullptr;
}
bool Exec::isBusy() {
    return rs != nullptr && state != 0;
}
bool Exec::valIsReady() {
    return rs != nullptr && state == delay;
}
Cdb Exec::getVal() {
    return Cdb {
        .value = rs->execute(),
        .tag = rs->tag,
    };
}

ResStation* Exec::getInstr() {
    return rs;
}