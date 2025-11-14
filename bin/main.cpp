#include <iostream>
#include <lib/argparser.h>

struct Options {
    bool sum = false;
    bool mult = false;
};

bool IsEven(const int& value) {
    return value % 2 == 0;
}

int main(int argc, const char** argv) {
    return 0;
}