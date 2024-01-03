#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <map>
#include "linux-perf-events.h"
#include "mini_perf.hpp"
#include "mini_perf_macro.hpp"
#include "utilities.hpp"

using ull = unsigned long long;
using ClockType = typename std::conditional<std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
using ClockTimePointType = std::chrono::steady_clock::time_point;
using ClockDurationType = std::chrono::steady_clock::duration;

template<typename TimeDurationType>
struct MiniPerf {
    const std::string perf_name;

    // Variables
    ClockTimePointType start_time;
    ClockDurationType time_count{};
    double average_ipc{};
    std::tuple<int, int, double> cpu_usage; // user, system, usage
    std::vector<int> mini_attribute_metrics;
    std::vector<ull> mini_attribute_start;
    std::vector<ull> mini_attribute_count;
    LinuxEvents<> perf_events;
    std::vector<int> perf_attribute_metrics;
    std::vector<ull> perf_attribute_start;
    std::vector<ull> perf_attribute_count;
    std::map<std::string, std::string> custom_metrics;

    // Methods
    explicit MiniPerf(const std::vector<int> &mini_parameters = {MINI_TIME_COUNT},
                      const std::vector<int> &perf_parameters = {},
                      std::string perf_name = "Mini Perf");

    MiniPerf(const MiniPerf &) = delete;

    MiniPerf(MiniPerf &&) = delete;

    void start();

    void stop();

    void add_custom_metric(const std::string &metric_name, const std::string &metric_value) {
        custom_metrics[metric_name] = metric_value;
    }

    void remove_custom_metric(const std::string &metric_name) {
        custom_metrics.erase(metric_name);
    }

    void reset();

    void report(const std::string &report_name = "Mini-Perf Report", bool to_file = false, bool to_stdout = true,
                const std::string &file_path = "./mini_perf_report.log");

    void report_in_row(const std::string &report_name = "Mini-Perf Report", bool to_file = false, bool to_stdout = true,
                       const std::string &file_path = "./mini_perf_report.csv",
                       const std::string &delimiter = ",");
};

// Implementations -------------------------------------
template<typename TimeDurationType>
MiniPerf<TimeDurationType>::MiniPerf(const std::vector<int> &mini_parameters, const std::vector<int> &perf_parameters,
                                     std::string perf_name): perf_name(std::move(perf_name)),
                                                             mini_attribute_metrics(mini_parameters),
                                                             perf_attribute_metrics(perf_parameters),
                                                             perf_events(perf_parameters) {
    // Sort and check mini parameters
    int ptr = 0;
    for (auto metric: mini_attribute_metrics) {
        if (metric > MINI_ATTRIBUTE_MAX) {
            throw (std::invalid_argument("Invalid mini parameter: " + std::to_string(metric)));
        }
        if (metric == MINI_CACHE_MISS_RATE) {
            // Must have perf parameter: PERF_COUNT_HW_CACHE_REFERENCES, PERF_COUNT_HW_CACHE_MISSES
            if (std::find(perf_attribute_metrics.begin(), perf_attribute_metrics.end(),
                          PERF_COUNT_HW_CACHE_REFERENCES) == perf_attribute_metrics.end() ||
                std::find(perf_attribute_metrics.begin(), perf_attribute_metrics.end(),
                          PERF_COUNT_HW_CACHE_MISSES) == perf_attribute_metrics.end()) {
                throw (std::invalid_argument("Invalid mini parameter " + std::to_string(metric) + ": need dependency"));
            }
        }
        if (metric == MINI_BRANCH_MISS_RATE) {
            // Must have perf parameter: PERF_COUNT_HW_BRANCH_INSTRUCTIONS, PERF_COUNT_HW_BRANCH_MISSES
            if (std::find(perf_attribute_metrics.begin(), perf_attribute_metrics.end(),
                          PERF_COUNT_HW_BRANCH_INSTRUCTIONS) == perf_attribute_metrics.end() ||
                std::find(perf_attribute_metrics.begin(), perf_attribute_metrics.end(),
                          PERF_COUNT_HW_BRANCH_MISSES) == perf_attribute_metrics.end()) {
                throw (std::invalid_argument("Invalid mini parameter " + std::to_string(metric) + ": need dependency"));
            }
        }
        if (metric == MINI_AVERAGE_IPC) {
            // Must have perf parameter: PERF_COUNT_HW_BRANCH_INSTRUCTIONS, PERF_COUNT_HW_BRANCH_MISSES
            if (std::find(perf_attribute_metrics.begin(), perf_attribute_metrics.end(),
                          PERF_COUNT_HW_CPU_CYCLES) == perf_attribute_metrics.end() ||
                std::find(perf_attribute_metrics.begin(), perf_attribute_metrics.end(),
                          PERF_COUNT_HW_INSTRUCTIONS) == perf_attribute_metrics.end()) {
                throw (std::invalid_argument("Invalid mini parameter " + std::to_string(metric) + ": need dependency"));
            }
        }
        ptr += 1;
    }

    // Perf results
    perf_attribute_start.resize(perf_attribute_metrics.size());
    perf_attribute_count.resize(perf_attribute_metrics.size());

    // Mini results
    mini_attribute_start.resize(mini_attribute_metrics.size());
    mini_attribute_count.resize(mini_attribute_metrics.size());
    ptr = 0;
    for (auto metric: mini_attribute_metrics) {
        if (metric == MINI_TIME_COUNT) {
            time_count = ClockDurationType::zero();
        } else {
            mini_attribute_start[ptr] = 0;
            mini_attribute_count[ptr] = 0;
        }
        ptr += 1;
    }

    this->reset();
}

template<typename TimeDurationType>
void MiniPerf<TimeDurationType>::start() {
    // Perf results
    if (!perf_attribute_metrics.empty()) {
        perf_events.start();
    }

    // Mini results
    int ptr = 0;
    for (auto metric: mini_attribute_metrics) {
        if (metric == MINI_TIME_COUNT) {
            start_time = ClockType::now();
        } else if (metric == MINI_MEMORY_COUNT) {
            double vm, rss;
            process_mem_usage(vm, rss);
            mini_attribute_start[ptr] = rss;
        } else if (metric == MINI_MEMORY_TOTAL) {
            double vm, rss;
            process_mem_usage(vm, rss);
            mini_attribute_start[ptr] = rss;
        } else if (metric == MINI_CPU_UTILIZATION) {
            process_cpu_utilization(cpu_usage);
        }
        ptr += 1;
    }
}

template<typename TimeDurationType>
void MiniPerf<TimeDurationType>::stop() {
    // Perf results
    if (!perf_attribute_metrics.empty()) {
        perf_events.end(perf_attribute_start);
        for (int i = 0; i < perf_attribute_count.size(); ++i) {
            perf_attribute_count[i] += perf_attribute_start[i];
        }
    }

    // Mini results
    int ptr = 0;
    for (auto metric: mini_attribute_metrics) {
        if (metric == MINI_TIME_COUNT) {
            time_count += ClockType::now() - start_time;
        } else if (metric == MINI_MEMORY_COUNT) {
            double vm, rss;
            process_mem_usage(vm, rss);
            mini_attribute_count[ptr] += rss - mini_attribute_start[ptr];
        } else if (metric == MINI_MEMORY_TOTAL) {
            mini_attribute_count[ptr] = mini_attribute_start[ptr];
        } else if (metric == MINI_CACHE_MISS_RATE) {
            int miss_ptr = 0, ref_ptr = 0;
            for (int i = 0; i < perf_attribute_metrics.size(); ++i) {
                if (perf_attribute_metrics[i] == PERF_COUNT_HW_CACHE_REFERENCES) {
                    ref_ptr = i;
                } else if (perf_attribute_metrics[i] == PERF_COUNT_HW_CACHE_MISSES) {
                    miss_ptr = i;
                }
            }
            int miss = perf_attribute_start[miss_ptr];
            int ref = perf_attribute_start[ref_ptr];
            mini_attribute_count[ptr] = static_cast<double>(miss * 100) / static_cast<double>(ref);
        } else if (metric == MINI_BRANCH_MISS_RATE) {
            int miss_ptr = 0, inst_ptr = 0;
            for (int i = 0; i < perf_attribute_metrics.size(); ++i) {
                if (perf_attribute_metrics[i] == PERF_COUNT_HW_BRANCH_INSTRUCTIONS) {
                    inst_ptr = i;
                } else if (perf_attribute_metrics[i] == PERF_COUNT_HW_BRANCH_MISSES) {
                    miss_ptr = i;
                }
            }
            int miss = perf_attribute_start[miss_ptr];
            int inst = perf_attribute_start[inst_ptr];
            mini_attribute_count[ptr] = static_cast<double>(miss * 100) / static_cast<double>(inst);
        } else if (metric == MINI_AVERAGE_IPC) {
            int cycle_ptr = 0, inst_ptr = 0;
            for (int i = 0; i < perf_attribute_metrics.size(); ++i) {
                if (perf_attribute_metrics[i] == PERF_COUNT_HW_CPU_CYCLES) {
                    cycle_ptr = i;
                } else if (perf_attribute_metrics[i] == PERF_COUNT_HW_INSTRUCTIONS) {
                    inst_ptr = i;
                }
            }
            int cycle = perf_attribute_start[cycle_ptr];
            int inst = perf_attribute_start[inst_ptr];
            average_ipc = static_cast<double>(inst) / static_cast<double>(cycle);
        } else if (metric == MINI_CPU_UTILIZATION) {
            process_cpu_utilization(cpu_usage);
            mini_attribute_count[ptr] = std::get<2>(cpu_usage);
        }
        ptr += 1;
    }
}

template<typename TimeDurationType>
void MiniPerf<TimeDurationType>::reset() {
    // Perf results
    if (!perf_attribute_metrics.empty()) {
        std::fill(perf_attribute_start.begin(), perf_attribute_start.end(), 0);
        std::fill(perf_attribute_count.begin(), perf_attribute_count.end(), 0);
    }

    // Mini results
    int ptr = 0;
    time_count = ClockDurationType::zero();
    average_ipc = 0;
    cpu_usage = {0, 0, 0.0};
    std::fill(mini_attribute_start.begin(), mini_attribute_start.end(), 0);
    std::fill(mini_attribute_count.begin(), mini_attribute_count.end(), 0);

    // Custom metrics
    custom_metrics.clear();
}

template<typename TimeDurationType>
void MiniPerf<TimeDurationType>::report(const std::string &report_name, bool to_file, bool to_stdout,
                                        const std::string &file_path) {
    std::ofstream file;
    if (to_file) {
        file = std::ofstream(file_path, std::ios::app);
    }
    int ptr = 0;
    // Report name
    auto perf_name_msg = "Perf Name: " + perf_name;
    log_println(perf_name_msg, to_stdout, to_file, file);
    auto report_name_msg = "Report Name: " + report_name;
    log_println(report_name_msg, to_stdout, to_file, file);
    auto report_time_msg = "Report Time: " + get_time();
    log_println(report_time_msg, to_stdout, to_file, file);
    // Mini results
    ptr = 0;
    for (auto metric: mini_attribute_metrics) {
        if (metric == MINI_TIME_COUNT) {
            auto duration = std::chrono::duration_cast<TimeDurationType>(time_count).count();
            auto msg =
                    get_mini_metric_name(metric) + ": " + std::to_string(duration) + get_time_unit<TimeDurationType>();
            log_println(msg, to_stdout, to_file, file);
        } else if (metric == MINI_AVERAGE_IPC) {
            auto msg = get_mini_metric_name(metric) + ": " + std::to_string(average_ipc) +
                       get_mini_metric_unit(metric);
            log_println(msg, to_stdout, to_file, file);
        } else {
            auto msg = get_mini_metric_name(metric) + ": " + std::to_string(mini_attribute_count[ptr]) +
                       get_mini_metric_unit(metric);
            log_println(msg, to_stdout, to_file, file);
        }
        ptr += 1;
    }

    // Perf results
    ptr = 0;
    for (auto metric: perf_attribute_metrics) {
        auto msg = get_perf_metric_name(metric) + ": " + std::to_string(perf_attribute_count[ptr]);
        log_println(msg, to_stdout, to_file, file);
        ptr += 1;
    }

    // Custom metrics
    for (auto &[metric_name, metric_value]: custom_metrics) {
        auto msg = metric_name + ": " + metric_value;
        log_println(msg, to_stdout, to_file, file);
    }

    if (to_file) {
        file.close();
    }
}

template<typename TimeDurationType>
void MiniPerf<TimeDurationType>::report_in_row(const std::string &report_name, bool to_file, bool to_stdout,
                                               const std::string &file_path, const std::string &delimiter) {
    std::ofstream file;
    if (to_file) {
        file = std::ofstream(file_path, std::ios::app);
    }
    int ptr = 0;

    // Print header if the file is empty
    if (file.tellp() == 0) {
        // Report info
        {
            auto perf_name_msg = "Perf Name" + delimiter;
            log_print(perf_name_msg, to_stdout, to_file, file);
            auto report_name_msg = "Report Name" + delimiter;
            log_print(report_name_msg, to_stdout, to_file, file);
            auto report_time_msg = "Report Time" + delimiter;
            log_print(report_time_msg, to_stdout, to_file, file);
        }
        // Mini metrics
        ptr = 0;
        for (auto metric: mini_attribute_metrics) {
            if (metric == MINI_TIME_COUNT) {
                auto msg =
                        get_mini_metric_name(metric) + "(" + get_time_unit<TimeDurationType>() + ")" + delimiter;
                log_print(msg, to_stdout, to_file, file);
            } else if (metric == MINI_AVERAGE_IPC) {
                auto msg = get_mini_metric_name(metric) + delimiter;
                log_print(msg, to_stdout, to_file, file);
            } else {
                auto msg = get_mini_metric_name(metric) + "(" + get_mini_metric_unit(metric) + ")" + delimiter;
                log_print(msg, to_stdout, to_file, file);
            }
            ptr += 1;
        }
        // Perf metrics
        ptr = 0;
        for (auto metric: perf_attribute_metrics) {
            auto msg = get_perf_metric_name(metric) + delimiter;
            log_print(msg, to_stdout, to_file, file);
            ptr += 1;
        }
        // Custom metrics
        for (auto &[metric_name, _]: custom_metrics) {
            auto msg = metric_name + delimiter;
            log_print(msg, to_stdout, to_file, file);
        }
        // End of metrics
        log_println("", to_stdout, to_file, file);
    }

    // Report info
    {
        auto perf_name_msg = perf_name + delimiter;
        log_print(perf_name_msg, to_stdout, to_file, file);
        auto report_name_msg = report_name + delimiter;
        log_print(report_name_msg, to_stdout, to_file, file);
        auto report_time_msg = get_time() + delimiter;
        log_print(report_time_msg, to_stdout, to_file, file);
    }
    // Mini results
    ptr = 0;
    for (auto metric: mini_attribute_metrics) {
        if (metric == MINI_TIME_COUNT) {
            auto duration = std::chrono::duration_cast<TimeDurationType>(time_count).count();
            auto msg = std::to_string(duration) + delimiter;
            log_print(msg, to_stdout, to_file, file);
        } else if (metric == MINI_AVERAGE_IPC) {
            auto msg = std::to_string(average_ipc)+ delimiter;
            log_print(msg, to_stdout, to_file, file);
        } else {
            auto msg = std::to_string(mini_attribute_count[ptr]) + delimiter;
            log_print(msg, to_stdout, to_file, file);
        }
        ptr += 1;
    }
    // Perf results
    ptr = 0;
    for (auto metric: perf_attribute_metrics) {
        auto msg = std::to_string(perf_attribute_count[ptr]) + delimiter;
        log_print(msg, to_stdout, to_file, file);
        ptr += 1;
    }
    // Custom metrics
    for (auto &[metric_name, metric_value]: custom_metrics) {
        auto msg = metric_value + delimiter;
        log_print(msg, to_stdout, to_file, file);
    }
    // End of metrics
    log_println("", to_stdout, to_file, file);

    if (to_file) {
        file.close();
    }
}
