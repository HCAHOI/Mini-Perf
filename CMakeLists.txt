cmake_minimum_required(VERSION 3.10)
project(mini_perf)

add_compile_options(-O3 -w)
set(CMAKE_CXX_STANDARD 20)

add_library(mini_perf INTERFACE src/mini_perf.cpp include/mini_perf.hpp include/mini_perf_macro.hpp include/utilities.hpp include/linux-perf-events.h)