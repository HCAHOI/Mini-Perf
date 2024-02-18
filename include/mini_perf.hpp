#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <map>
#include <filesystem>
#include "linux-perf-events.h"
#include "mini_perf.hpp"
#include "mini_perf_macro.hpp"
#include "utilities.hpp"

using ull = unsigned long long;
using ClockType = typename std::conditional<std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
using ClockTimePointType = std::chrono::steady_clock::time_point;
using ClockDurationType = std::chrono::steady_clock::duration;

template<typename TimeDurationType = std::chrono::milliseconds>
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

    void reset();

    void report(const std::string &report_name = "Mini-Perf Report", bool to_file = false, bool to_stdout = true,
                const std::string &file_path = "./mini_perf_report.log");

    void report_in_row(const std::string &report_name = "Mini-Perf Report", bool to_file = false, bool to_stdout = true,
                       const std::string &file_path = "./mini_perf_report.csv",
                       const std::string &delimiter = ",");

    auto get_time_count() {
        return std::chrono::duration_cast<TimeDurationType>(time_count);
    }

    void metrics_average(size_t iterations);

    void add_custom_metric(const std::string &metric_name, const std::string &metric_value);

    void remove_custom_metric(const std::string &metric_name);
};
