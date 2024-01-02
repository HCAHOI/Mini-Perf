//
// Created by hoi on 24-1-2.
//
#pragma once

/// Macro for print_log file and line info in report.
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
