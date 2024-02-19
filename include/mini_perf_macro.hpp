//
// Created by hoi on 24-1-2.
//
#pragma once

/// Macro for log_println file and line info in report.
#define PerfReport(perf, report_name, to_file, to_stdout, file_path)              \
    if (to_stdout) {                                    \
        std::cout << "----------------------------------------" << std::endl;         \
        std::cout << "Report at " <<  __FILE__ << ": Line " << __LINE__ << std::endl;     \
    }                                                \
    if (to_file) {                                      \
        std::ofstream file(file_path, std::ios::app);   \
        file << "----------------------------------------" << std::endl;    \
        file << "Report at " <<  __FILE__ << ": Line " << __LINE__ << std::endl;     \
        file.close();                                                    \
    }                                                   \
    perf.report(report_name, to_file, to_stdout, file_path); \
    if (to_stdout) {                                    \
        std::cout << "----------------------------------------" << std::endl;         \
    }                                                \
    if (to_file) {                                      \
        std::ofstream file(file_path, std::ios::app);   \
        file << "----------------------------------------" << std::endl;   \
        file.close();                                                    \
    }

/// Macro for report a collection of metrics in a row.
#define PerfReportInRow(perf, report_name, to_file, to_stdout, file_path)              \
    if (to_stdout) {                                    \
        std::cout << "----------------------------------------" << std::endl;         \
        std::cout << "Report at " <<  __FILE__ << ": Line " << __LINE__ << std::endl;     \
    }                                                \
    perf.report_in_row(report_name, to_file, to_stdout, file_path); \
    if (to_stdout) {                                    \
        std::cout << "----------------------------------------" << std::endl;         \
    }                                                                                  \


/// Macro for Mini Perf's Unit Benchmark. The initialization part should be done between the
/// MiniInit and MiniEnd. The main part that you want to benchmark should
/// be done between the MiniUnitStart and MiniUnitEnd. max_iteration_time's unit is second.
#define MiniInit(perf_name, mini_metrics, perf_metrics, max_time)  \
{                                      \
    auto perf = mperf::MiniPerf<std::chrono::microseconds>{mini_metrics, perf_metrics, perf_name}; \
    std::chrono::microseconds cur_time = std::chrono::microseconds{0};     \
    std::chrono::microseconds max_iteration_time = std::chrono::microseconds{max_time * 1000000}; \
    size_t iterations = 0;                      \


#define MiniUnitStart          \
    {                                      \        
        while(true) {                   \
            perf.start();


#define MiniUnitEnd(report_name, tofile, report_path) \
            perf.stop();                            \
            cur_time = perf.get_time_count();  \
            iterations += 1;          \
            if (cur_time > max_iteration_time) break; \
        }                \
        perf.metrics_average(iterations);                         \
        perf.add_custom_metric("Iterations", std::to_string(iterations)); \
        PerfReportInRow(perf, report_name, tofile, true, report_path)    \
        perf.reset(); \
    }                                      \


#define MiniEnd }
