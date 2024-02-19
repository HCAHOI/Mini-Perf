# Mini Perf

## Introduction

Mini Perf is a small Linux C++ program performance measurement tool. It can be easily used. 

## TODO

- [x] Basic functions
- [x] Export to CSV
- [x] Benchmark
- [x] Quick Timer
- [x] Optimize Benchmark

## Supported Metrics

### Mini Perf Metrics

* MINI_TIME_COUNT

  A timer.

* MINI_MEMORY_COUNT

  Allocated physical memory during the Mini Perf is running.

* MINI_MEMORY_TOTAL

  The process physical memory when the `stop()` is called.

* MINI_CACHE_MISS_RATE

  The cache miss rate of all caches.

* MINI_BRANCH_MISS_RATE

  The prediction failure rate of branches.

* MINI_AVERAGE_IPC

  Average IPC.

* MINI_CPU_UTILIZATION

  The CPU utilization rate of the process. 

### Linux Perf Metrics

* PERF_COUNT_HW_CPU_CYCLES
* PERF_COUNT_HW_INSTRUCTIONS 
* PERF_COUNT_HW_CACHE_REFERENCES
* PERF_COUNT_HW_CACHE_MISSES
* PERF_COUNT_HW_BRANCH_INSTRUCTIONS
* PERF_COUNT_HW_BRANCH_MISSES 
* PERF_COUNT_HW_BUS_CYCLES
* PERF_COUNT_HW_STALLED_CYCLES_FRONTEND
* PERF_COUNT_HW_STALLED_CYCLES_BACKEND
* PERF_COUNT_HW_REF_CPU_CYCLES 

### Supported Duration Type

*  std::chrono::nanoseconds
* std::chrono::microseconds
* std::chrono::milliseconds
* std::chrono::seconds
* std::chrono::minutes
* std::chrono::hours

## Install

Mini Perf recommends using Xmake for compilation and linking, and we provide build scripts for make and cmake in the `other_install` directory. Note that Mini Perf requires c++20 or higher. 

## How to Use

### Mini Timer

The Mini Timer has been simplified as much as possible to minimize the complexity of using it.

```cpp
#include <mini_perf.hpp>

mperf::Timer timer;   // or mperf::Timer timer{timer_name}

timer.start();
// do something
timer.stop();

```


### Mini Perf

```cpp
#include "mini_perf.hpp"

using namespace mperf;

// Define metrics to be measured.
std::vector<int> mini_metrics = {MINI_TIME_COUNT, MINI_AVERAGE_IPC, MINI_CPU_UTILIZATION , MINI_CACHE_MISS_RATE};
std::vector<int> perf_metrics = {PERF_COUNT_HW_CPU_CYCLES, PERF_COUNT_HW_INSTRUCTIONS, PERF_COUNT_HW_CACHE_MISSES, PERF_COUNT_HW_CACHE_REFERENCES};

// MiniPerf<TimeType>(mini_parameters, perf_parameters, perf_name);
mperf::MiniPerf<std::chrono::microseconds> perf{mini_metrics, perf_metrics, "Mini Perf 1"};

perf.start();
// do something...
perf.stop();

// report(report_name, to_file, to_stdout, file_path)
perf.report();
// Or use the macro to get more information.
// PerfReport(perf_instance, report_name, to_file, to_stdout, log_file_path)
PerfReport(perf, "Report test1", true, true, "./perf.log");

perf.reset();

perf.start();
// do something...
perf.stop();

PerfReport(perf, "Report test2", true, true, "./perf.log");
/*
----------------------------------------
Report at /home/hoi/projects/test/main.cpp: Line 58
Name: Mini Perf 1
Report Name: Report test2
Report Time: 2024/1/2 20:50:41
Running Time: 10001872 us
Average IPC: 0.289637
CPU Utilization: 92 %
Cache Miss Rate: 35 %
CPU Cycles: 304119
Instructions: 88084
Cache Misses: 7912
Cache References: 22050
----------------------------------------
*/
// PerfReportInRow will report the data in table format.
PerfReportInRow(perf, "Report test2", true, true, "./perf.csv");
```

### Mini-Benchmark

Mini-benchmark will execute the code between `MiniUnitStart` and `MiniUnitEnd` enough times(less than `max_running_time`) and output the average result.

```cpp
#include "mini_perf.hpp"
#include "mini_perf_macro.hpp"

// MiniInit(perf_name, mini_metrics, perf_metrics, max_running_time), note that the unit of the 'max_running_time' is second.
MiniInit("Micro Test", {MINI_TIME_COUNT}, {}, 1)
    // Initialization
    float arr[N];
MiniUnitStart
    // Main part to be measured 
    for(size_t i = 0; i < N; i++) {
        arr[i] = 1.0f;
    }
// MiniUnitEnd(report_name, report_file_path)
MiniUnitEnd("Test report1", "micro_test.csv")
MiniUnitStart
    for(size_t i = 0; i < N; i++) {
        arr[i] = 0.0f;
    }
MiniUnitEnd("Test report2", "micro_test.csv")
MiniEnd
```

## Notes

* Mini Perf counts the average metrics of all intervals. If you want to measure the metrics for each interval separately, call `reset()` before the next `start()`.
* Considering the running of the instance itself, we suggest that the computation between `start()` and `stop` or in the micro-benchmark should be complex enough. Or you can decrease the number of metrics.
* Use one instance in several threads will produce invalid data.
