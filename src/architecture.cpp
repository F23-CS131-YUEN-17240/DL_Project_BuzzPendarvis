#include "architecture.hpp"
#include <algorithm>

void Rob::reset() {
    head = 2;
    tail = 1;
    for (int i = 0; i < 16; i++) rob[i].ready = false;
    empty = true;
    full = false;
}
bool Rob::isFull() {
    return full;
}
bool Rob::isEmpty() {
    return empty;
}
bool Rob::isReady() {
    return !isEmpty() && rob[head].ready;
}
void Rob::enqueue(RobEntry in) {
    tail = std::max(1, (tail + 1) & 0xF);
    rob[tail] = in;
    empty = false;
    if (std::max(1, (tail + 1) & 0xF) == head) full = true;
}
RobEntry Rob::dequeue() {
    RobEntry out = rob[head];
    head = std::max(1, (head + 1) & 0xF);
    full = false;
    if (std::max(1, (head + 1) & 0xF) == tail) empty = true;
    return out;
}
uint8_t Rob::tagc(uint8_t tag) {
    return tag + (tag < head) * 16;
}