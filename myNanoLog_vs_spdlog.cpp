#include <bits/stdc++.h>
#include "NanoLog.hpp"
#include "spdlog/spdlog.h"

template <typename Function>
void run_log_benchmark(Function&& f, char const* const logger) {
    const int iterations = 1E5;
    std::vector<uint64_t> latencies(iterations);
    char const* const benchmark = "benchmark";
    uint64_t st, en;
    for (int i = 0; i < iterations; ++i) {
        st = timestamp_now();
        f(i, benchmark);
        en = timestamp_now();
        latencies[i] = en - st;
    }
    std::sort(latencies.begin(), latencies.end());
    uint64_t sum = std::accumulate(latencies.begin(), latencies.end(), (uint64_t)0);
    printf("%s [best, middle, worst, average] latency in microseconds\n%9s|%9s|%9s|%9s|\n%9ld|%9ld|%9ld|%9ld|%9ld|%9ld|%9lf|\n", logger, "Best", "Middle", "Worst", "Average", latencies[0], latencies[(size_t)iterations / 2], latencies.back(), (sum * 1.0) / latencies.size());
}

template <typename Function>
void run_benchmark(Function&& f, int thread_count, char const* const logger) {
    printf("\nThread count: %d\n", thread_count);
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(run_log_benchmark<Function>, std::ref(f), logger);
    }
    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }
}

void print_usage() {
    char const* const executable = "myNanoLog_vs_spdlog";
    printf("Usage \n1. %s nanolog\n2. %s spdlog\n", executable, executable);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "nanolog") == 0) {
        nanolog::initialize(nanolog::GuaranteedLogger(), "/home/wlx/myNanoLog/testfile/", "myNanoLog", 1);

        auto nanolog_benchmark = [](int i, char const* const cstr) { LOG_INFO << "Logging " << cstr << i << 0 << 'K' << -42.42; };
        for (auto threads : {1, 2, 3, 4})
            run_benchmark(nanolog_benchmark, threads, "nanolog_guaranteed");
    } else if (strcmp(argv[1], "spdlog") == 0) {
        spdlog::set_async_mode(1048576);
        auto spd_logger = spdlog::create<spdlog::sinks::simple_file_sink_mt>("file_logger", "/home/wlx/myNanoLog/testfile/spd-async.txt", false);

        auto spdlog_benchmark = [&spd_logger](int i, char const* const cstr) { spd_logger->info("Logging {}{}{}{}{}", cstr, i, 0, 'K', -42.42); };
        for (auto threads : {1, 2, 3, 4})
            run_benchmark(spdlog_benchmark, threads, "spdlog");
    }
}