#include <mini_perf.hpp>

#include <iostream>
#include <mini_perf.hpp>
#include <mini_perf_macro.hpp>
#include <cmath>
#include <immintrin.h>

using namespace std;

const size_t N = 1 << 20;

int main() {
    mperf::Timer timer;
    timer.start();

    MiniInit("Test", {mperf::MINI_TIME_COUNT}, {}, 1)
        volatile float arr[N];
    MiniUnitStart
        for(size_t i = 0; i < N; i++) {
            arr[i] = 1.0f;
        }
    MiniUnitEnd("Test report1", "micro_test.csv")
    MiniUnitStart
        for(size_t i = 0; i < N; i++) {
            arr[i] = 0.0f;
        }
    MiniUnitEnd("Test report2", "micro_test.csv")
    MiniEnd

    timer.stop();

    return 0;
}
