#include "mini_perf.hpp"
#include "mini_perf_macro.hpp"
#include "utilities.hpp"
#include <chrono>
#include <cmath>
#include <linux/perf_event.h>

using namespace mperf;

int main() {
    const size_t N = 1000000;

    std::vector<int> mini_metrics = {MINI_TIME_COUNT, MINI_CPU_UTILIZATION};
    std::vector<int> perf_metrics = {PERF_COUNT_HW_BRANCH_MISSES};
    MiniInit("Benchmark", mini_metrics, perf_metrics, 1)
    volatile float arr[N];
    MiniUnitStart
        for(size_t i = 0; i < N; i++) {
            arr[i] = i;
        }
    MiniUnitEnd("Benchmark Report1", true, "bencmark_sample.csv")
    MiniUnitStart
        for(size_t i = 0; i < N; i++) {
            arr[i] = std::sin(i);
        }
    MiniUnitEnd("Benchmark Report2", true, "bencmark_sample.csv")
    MiniEnd

    return 0;
}