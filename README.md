# Mini Perf

## Introduction

Mini Perf is a small Linux program performance measurement tool. It can be easily used after including the header file. 

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

## How to Use

```cpp
#include "mini_perf.hpp"

// ...

// Define metrics to be measured.
std::vector<int> mini_metrics = {MINI_TIME_COUNT, MINI_AVERAGE_IPC, MINI_CPU_UTILIZATION , MINI_CACHE_MISS_RATE};
std::vector<int> perf_metrics = {PERF_COUNT_HW_CPU_CYCLES, PERF_COUNT_HW_INSTRUCTIONS, PERF_COUNT_HW_CACHE_MISSES, PERF_COUNT_HW_CACHE_REFERENCES};

// MiniPerf<TimeType>(mini_parameters, perf_parameters, perf_name);
MiniPerf<std::chrono::microseconds> perf{mini_metrics, perf_metrics, "Mini Perf 1"};

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
Perf Name: Mini Perf 1
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
```

## Notes

* Mini Perf counts the average metrics of all intervals. If you want to measure the metrics for each interval separately, call `reset()` before the next `start()`.
* Too short interval might cause exception.
* Use one instance in several threads will produce invalid data.
