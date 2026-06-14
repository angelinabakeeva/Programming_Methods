#include <iostream>
#include <cstdint>

class MidSquareFixed {
    uint32_t state;
public:
    MidSquareFixed(uint32_t seed) {
        seed = seed % 100000000;
        if (seed < 10000000) seed += 10000000;
        state = seed;
    }
    uint32_t next() {
        uint64_t sq = (uint64_t)state * state;
        state = (sq / 10000) % 100000000;
        if (state < 10000000) state ^= 0x9E3779B9;
        state = state % 100000000;
        if (state < 10000000) state = 12345678;
        return state;   // возвращаем uint32_t, а не double
    }
};

class LCG {
    uint32_t state;
    static const uint32_t a = 1664525;
    static const uint32_t c = 1013904223;
public:
    LCG(uint32_t seed) : state(seed) {}
    uint32_t next() {
        state = a * state + c;
        uint32_t x = state;
        x ^= (x >> 16);
        return x;
    }
};

class XorshiftFixed {
    uint32_t state;
public:
    XorshiftFixed(uint32_t seed) : state(seed ? seed : 1) {}
    uint32_t next() {
        state ^= state << 7;
        state ^= state >> 9;
        state ^= state << 13;
        state = state * 0x2545F491;
        return state;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) return 1;
    int method = atoi(argv[1]);  // 1, 2 или 3
    uint32_t seed = 12345;
    const uint64_t N = 5000000;  // 5 млн чисел = 20 МБ, для тестов хватит

    if (method == 1) {
        MidSquareFixed gen(seed);
        for (uint64_t i = 0; i < N; ++i) {
            uint32_t val = gen.next();
            std::cout.write((char*)&val, 4);
        }
    } else if (method == 2) {
        LCG gen(seed);
        for (uint64_t i = 0; i < N; ++i) {
            uint32_t val = gen.next();
            std::cout.write((char*)&val, 4);
        }
    } else if (method == 3) {
        XorshiftFixed gen(seed);
        for (uint64_t i = 0; i < N; ++i) {
            uint32_t val = gen.next();
            std::cout.write((char*)&val, 4);
        }
    }
    return 0;
}