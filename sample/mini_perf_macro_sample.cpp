#include "mini_perf.hpp"
#include "mini_perf_macro.hpp"
#include "utilities.hpp"
#include <chrono>
#include <cmath>
#include <linux/perf_event.h>

using namespace mperf;

int main() {
    const size_t N = 1000000;
    volatile float arr[N];
    
    // Simple usage
    MiniPerf mp1;

    mp1.start();
    for(size_t i = 0; i < N; i++) {
        arr[i] = i;
    }
    mp1.stop();
    PerfReport(mp1, "MiniPerf1 Report", false, true, "");

    // Full Instantiation
    MiniPerf<std::chrono::microseconds> mp2({MINI_TIME_COUNT, MINI_CPU_UTILIZATION}, {PERF_COUNT_HW_BRANCH_MISSES}, "Sample MiniPerf2");

    mp2.start();
    for(size_t i = 0; i < N; i++) {
        arr[i] = i;
    }
    mp2.stop();
    PerfReportInRow(mp2, "MiniPerf2 Report", true, true, "sample.csv");
    PerfReportInRow(mp2, "MiniPerf2 Report", true, true, "sample.csv");
    PerfReportInRow(mp2, "MiniPerf2 Report", true, true, "sample.csv");

    return 0;
}