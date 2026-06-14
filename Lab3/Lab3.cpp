#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <cstdint>
#include <chrono>
#include <random>

class MidSquareFixed {
    uint32_t state;
public:
    MidSquareFixed(uint32_t seed) {
        seed = seed % 100000000;
        if (seed < 10000000) seed += 10000000;
        state = seed;
    }
    double next() {
        uint64_t sq = (uint64_t)state * state;
        state = (sq / 10000) % 100000000;
        if (state < 10000000) state ^= 0x9E3779B9;
        state = state % 100000000;
        if (state < 10000000) state = 12345678;
        return state / 100000000.0;
    }
};

class LCG {
    uint32_t state;
    static const uint32_t a = 1664525;
    static const uint32_t c = 1013904223;
public:
    LCG(uint32_t seed) : state(seed) {}
    double next() {
        state = a * state + c;
        uint32_t x = state;
        x ^= (x >> 16);
        return x / 4294967296.0;
    }
};

class XorshiftFixed {
    uint32_t state;
public:
    XorshiftFixed(uint32_t seed) : state(seed ? seed : 1) {}
    double next() {
        state ^= state << 7;
        state ^= state >> 9;
        state ^= state << 13;
        state = state * 0x2545F491;
        return state / 4294967296.0;
    }
};

struct Stats {
    double mean, stddev, cv, chi2;
    bool uniform;
};

Stats computeStats(const std::vector<double>& data, int bins = 20) {
    size_t n = data.size();
    double sum = 0.0;
    for (double v : data) sum += v;
    double mean = sum / n;

    double sumSq = 0.0;
    for (double v : data) sumSq += (v - mean) * (v - mean);
    double variance = sumSq / n;
    double stddev = sqrt(variance);
    double cv = stddev / mean;

    double expected = (double)n / bins;
    std::vector<int> counts(bins, 0);
    for (double v : data) {
        int bin = (int)(v * bins);
        if (bin >= bins) bin = bins - 1;
        counts[bin]++;
    }
    double chi2 = 0.0;
    for (int c : counts) {
        double diff = c - expected;
        chi2 += diff * diff / expected;
    }
    bool uniform = (chi2 < 30.14);
    return {mean, stddev, cv, chi2, uniform};
}

template<typename Gen>
double measureTime(Gen& gen, int n) {
    auto start = std::chrono::high_resolution_clock::now();
    volatile double tmp;
    for (int i = 0; i < n; ++i) {
        tmp = gen.next();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}

int main() {
    const int SAMPLES = 20;
    const int SIZE = 5000;
    const int BINS = 20;

    std::vector<uint32_t> seeds = {
        12345, 67890, 11111, 22222, 33333, 44444, 55555, 66666, 77777, 88888,
        99999, 10101, 20202, 30303, 40404, 50505, 60606, 70707, 80808, 90909
    };

    std::ofstream report("report_fixed.txt");
    report << std::fixed << std::setprecision(6);

    auto process = [&](const std::string& name, auto genFactory) {
        report << "\n========== " << name << " ==========\n";
        double sumMean = 0, sumStd = 0, sumCV = 0;
        int uniformCnt = 0;

        for (int s = 0; s < SAMPLES; ++s) {
            auto gen = genFactory(seeds[s]);
            std::vector<double> data(SIZE);
            for (int i = 0; i < SIZE; ++i) data[i] = gen.next();

            Stats st = computeStats(data, BINS);
            sumMean += st.mean;
            sumStd += st.stddev;
            sumCV += st.cv;
            if (st.uniform) uniformCnt++;

            report << "Sample " << std::setw(2) << s+1
                   << " | mean=" << st.mean
                   << " | stddev=" << st.stddev
                   << " | cv=" << st.cv
                   << " | chi2=" << st.chi2
                   << " | uniform=" << (st.uniform ? "YES" : "NO") << "\n";
        }
        report << "\n--- AVERAGE ---\n";
        report << "Mean   = " << sumMean / SAMPLES << "\n";
        report << "Stddev = " << sumStd / SAMPLES << "\n";
        report << "CV     = " << sumCV / SAMPLES << "\n";
        report << "Passed chi-square: " << uniformCnt << " / " << SAMPLES << "\n";
    };

    process("Method 1: MidSquare (8-digit, XOR fix)", [](uint32_t seed) { return MidSquareFixed(seed); });
    process("Method 2: LCG + XOR shift", [](uint32_t seed) { return LCG(seed); });
    process("Method 3: Xorshift + multiply (fixed)", [](uint32_t seed) { return XorshiftFixed(seed); });

    report.close();
    std::cout << "Statistics saved to report_fixed.txt\n";

    std::vector<int> sizes = {1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    std::vector<double> time1, time2, time3, time_std;
    uint32_t seed_time = 12345;

    // Метод 1
    for (int n : sizes) {
        MidSquareFixed gen(seed_time);
        time1.push_back(measureTime(gen, n));
    }
    // Метод 2
    for (int n : sizes) {
        LCG gen(seed_time);
        time2.push_back(measureTime(gen, n));
    }
    // Метод 3
    for (int n : sizes) {
        XorshiftFixed gen(seed_time);
        time3.push_back(measureTime(gen, n));
    }
    // Стандартный метод C++ (mt19937)
    std::mt19937 std_gen(seed_time);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int n : sizes) {
        auto start = std::chrono::high_resolution_clock::now();
        volatile double tmp;
        for (int i = 0; i < n; ++i) {
            tmp = dist(std_gen);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        time_std.push_back(elapsed.count());
    }

    // Сохраняем в CSV
    std::ofstream csv("times.csv");
    csv << "size,method1,method2,method3,std_mt19937\n";
    for (size_t i = 0; i < sizes.size(); ++i) {
        csv << sizes[i] << "," << time1[i] << "," << time2[i] << "," << time3[i] << "," << time_std[i] << "\n";
    }
    csv.close();
    std::cout << "Timing results saved to times.csv\n";

    return 0;
}