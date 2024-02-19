#pragma once

#include <iostream>
#include <chrono>
#include <type_traits>
#include <linux/perf_event.h>

namespace mperf {
    const size_t MINI_ATTRIBUTE_MAX = 6;    // Do not forget to change this when adding new mini attributes.
    enum MiniFlag {
        MINI_TIME_COUNT = 0,
        MINI_MEMORY_COUNT = 1,  // Allocated physical mem. between start and stop.
        MINI_MEMORY_TOTAL = 2,  // Total process physical mem. when stopped.
        MINI_CACHE_MISS_RATE = 3,
        MINI_BRANCH_MISS_RATE = 4,
        MINI_AVERAGE_IPC = 5,
        MINI_CPU_UTILIZATION = 6,
    };

    std::string get_time() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        auto min = ltm->tm_min;
        auto min_str = (min < 10) ? "0" + std::to_string(min) : std::to_string(min);
        auto sec = ltm->tm_sec;
        auto sec_str = (sec < 10) ? "0" + std::to_string(sec) : std::to_string(sec);
        std::string time = std::to_string(1900 + ltm->tm_year) + "/" + std::to_string(1 + ltm->tm_mon) + "/" +
                        std::to_string(ltm->tm_mday) + " " + std::to_string(ltm->tm_hour) + ":" + min_str + ":" +
                        sec_str;
        return time;
    }

    /// Get the unit of the std::chrono duration types.
    template<typename T>
    std::string get_time_unit(int _ = 0) {
        if constexpr (std::is_same_v<T, std::chrono::hours>) {
            return "h";
        } else if constexpr (std::is_same_v<T, std::chrono::minutes>) {
            return "min";
        } else if constexpr (std::is_same_v<T, std::chrono::seconds>) {
            return "s";
        } else if constexpr (std::is_same_v<T, std::chrono::milliseconds>) {
            return "ms";
        } else if constexpr (std::is_same_v<T, std::chrono::microseconds>) {
            return "us";
        } else if constexpr (std::is_same_v<T, std::chrono::nanoseconds>) {
            return "ns";
        } else {
            std::cerr << "Unsupported time type." << std::endl;
            return "";
        }
    }

    void log_println(const std::string &msg, bool to_stdout, bool to_file, std::ofstream &file) {
        if (to_stdout) {
            std::cout << msg << std::endl;
        }
        if (to_file) {
            file << msg << std::endl;
        }
    }

    void log_print(const std::string &msg, bool to_stdout, bool to_file, std::ofstream &file) {
        if (to_stdout) {
            std::cout << msg ;
        }
        if (to_file) {
            file << msg;
        }
    }

    std::string get_mini_metric_unit(int metric) {
        if (metric == MINI_TIME_COUNT) {
            std::cerr << "Use get_time_unit() instead." << std::endl;
            return "";
        } else if (metric == MINI_MEMORY_COUNT) {
            return "KB";
        } else if (metric == MINI_MEMORY_TOTAL) {
            return "KB";
        } else if (metric == MINI_CACHE_MISS_RATE) {
            return "%";
        } else if (metric == MINI_BRANCH_MISS_RATE) {
            return "%";
        } else if (metric == MINI_CPU_UTILIZATION) {
            return "%";
        } else {
            return "";
        }
    }

    std::string get_mini_metric_name(int metric) {
        if (metric == MINI_TIME_COUNT) {
            return "Running Time";
        } else if (metric == MINI_MEMORY_COUNT) {
            return "Memory Count";
        } else if (metric == MINI_MEMORY_TOTAL) {
            return "Memory Total";
        } else if (metric == MINI_CACHE_MISS_RATE) {
            return "Cache Miss Rate";
        } else if (metric == MINI_BRANCH_MISS_RATE) {
            return "Branch Miss Rate";
        } else if (metric == MINI_AVERAGE_IPC) {
            return "Average IPC";
        } else if (metric == MINI_CPU_UTILIZATION) {
            return "CPU Utilization";
        } else {
            return "Unknown";
        }
    }


    /*
    * 	PERF_COUNT_HW_CPU_CYCLES		= 0,
        PERF_COUNT_HW_INSTRUCTIONS		= 1,
        PERF_COUNT_HW_CACHE_REFERENCES		= 2,
        PERF_COUNT_HW_CACHE_MISSES		= 3,
        PERF_COUNT_HW_BRANCH_INSTRUCTIONS	= 4,
        PERF_COUNT_HW_BRANCH_MISSES		= 5,
        PERF_COUNT_HW_BUS_CYCLES		= 6,
        PERF_COUNT_HW_STALLED_CYCLES_FRONTEND	= 7,
        PERF_COUNT_HW_STALLED_CYCLES_BACKEND	= 8,
        PERF_COUNT_HW_REF_CPU_CYCLES		= 9,
    */
    std::string get_perf_metric_name(int metric) {
        if (metric == PERF_COUNT_HW_CPU_CYCLES) {
            return "CPU Cycles";
        } else if (metric == PERF_COUNT_HW_INSTRUCTIONS) {
            return "Instructions";
        } else if (metric == PERF_COUNT_HW_CACHE_REFERENCES) {
            return "Cache References";
        } else if (metric == PERF_COUNT_HW_CACHE_MISSES) {
            return "Cache Misses";
        } else if (metric == PERF_COUNT_HW_BRANCH_INSTRUCTIONS) {
            return "Branch Instructions";
        } else if (metric == PERF_COUNT_HW_BRANCH_MISSES) {
            return "Branch Misses";
        } else if (metric == PERF_COUNT_HW_BUS_CYCLES) {
            return "Bus Cycles";
        } else if (metric == PERF_COUNT_HW_STALLED_CYCLES_FRONTEND) {
            return "Stalled Cycles Frontend";
        } else if (metric == PERF_COUNT_HW_STALLED_CYCLES_BACKEND) {
            return "Stalled Cycles Backend";
        } else if (metric == PERF_COUNT_HW_REF_CPU_CYCLES) {
            return "Ref CPU Cycles";
        } else {
            return "Unknown";
        }
    }

    // From Don Wakefield in https://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-runtime-using-c
    void process_mem_usage(double &vm_usage, double &resident_set) {
        using std::ios_base;
        using std::ifstream;
        using std::string;

        vm_usage = 0.0;
        resident_set = 0.0;

        // 'file' stat seems to give the most reliable results
        //
        ifstream stat_stream("/proc/self/stat", ios_base::in);

        // dummy vars for leading entries in stat that we don't care about
        //
        string pid, comm, state, ppid, pgrp, session, tty_nr;
        string tpgid, flags, minflt, cminflt, majflt, cmajflt;
        string utime, stime, cutime, cstime, priority, nice;
        string O, itrealvalue, starttime;

        // the two fields we want
        //
        unsigned long vsize;
        long rss;

        stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                    >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                    >> utime >> stime >> cutime >> cstime >> priority >> nice
                    >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

        stat_stream.close();

        long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
        vm_usage = vsize / 1024.0;
        resident_set = rss * page_size_kb;
    }


    void process_cpu_utilization(std::tuple<int, int, double> &utilization) {
        using std::ios_base;
        using std::ifstream;
        using std::string;

        auto [p_start, s_start, usage] = utilization;
        utilization = std::make_tuple(0, 0, 0.0);


        // dummy vars for leading entries in stat that we don't care about
        //
        string pid, comm, state, ppid, pgrp, session, tty_nr;
        string tpgid, flags, minflt, cminflt, majflt, cmajflt;
        string cutime, cstime, priority, nice;
        string O, itrealvalue, starttime;

        // the fields we want
        size_t p_utime, p_stime;

        // 'file' stat seems to give the most reliable results
        //
        ifstream p_stat_stream("/proc/self/stat", ios_base::in);

        p_stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                    >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                    >> p_utime >> p_stime >> cutime >> cstime >> priority >> nice
                    >> O >> itrealvalue >> starttime; // don't care about the rest

        p_stat_stream.close();

        // read system cpu time
        // 'file' stat seems to give the most reliable results
        size_t s_utime, s_ntime, s_stime, s_itime, s_iotime, s_irq, s_sirq;

        ifstream s_stat_stream("/proc/stat", ios_base::in);

        s_stat_stream >> comm >> s_utime >> s_ntime >> s_stime >> s_itime >> s_iotime >> s_irq >> s_sirq;
        s_stat_stream.close();

        auto p_total_time = p_utime + p_stime;
        auto s_total_time = s_utime + s_ntime + s_stime + s_itime + s_iotime + s_irq + s_sirq;

        if (p_start == 0 && s_start == 0) {
            utilization = std::make_tuple(p_total_time, s_total_time, 0.0);
        } else {
            auto p_delta_time = p_total_time - p_start;
            auto s_delta_time = s_total_time - s_start;
            if (p_delta_time == 0 || s_delta_time == 0) {
                utilization = std::make_tuple(p_start, s_start, 0.0);
                return;
            }
            auto utilization_rate = p_delta_time * 100 / s_delta_time;
            utilization = std::make_tuple(p_start, s_start, utilization_rate);
        }
    }

}   // namespace mperf