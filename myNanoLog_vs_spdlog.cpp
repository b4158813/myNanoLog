#include <bits/stdc++.h>
#include "myNanoLog"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

// 获取当前时间，精确到ns
uint64_t timestamp_now() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

template <typename Function>
void run_log_benchmark(Function&& f, char const* const logger) {
    const int iterations = 1E5;
    std::vector<uint64_t> latencies(iterations);
    char const* const benchmark = "benchmark";
    uint64_t st, en;
    st = timestamp_now();
    for (int i = 0; i < iterations; ++i) {
        f(i, benchmark);
        // latencies[i] = en - st;
    }
    en = timestamp_now();
    std::sort(latencies.begin(), latencies.end());
    uint64_t sum = std::accumulate(latencies.begin(), latencies.end(), (uint64_t)0);

    std::cout << "Average latency = " << (1.0 * en - st) / iterations / 1000 << "μs\n";
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
    char const* const executable = "test_vs";
    printf("Usage \n1. %s myNanoLog\n2. %s spdlog\n", executable, executable);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "myNanoLog") == 0) {
        myNanoLog::initialize(myNanoLog::GuaranteedLogger(), "/home/wlx/myNanoLog/testfile/", "myNanoLog", 1);

        auto nanolog_benchmark = [](int i, char const* const cstr) { LOG_INFO << "Logging " << cstr << i << 0 << 'K' << -42.42; };
        for (auto threads : {1, 2, 3, 4, 5})
            run_benchmark(nanolog_benchmark, threads, "myNanoLog_GL");
    } else if (strcmp(argv[1], "spdlog") == 0) {
        auto spd_logger = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("file_logger", "/home/wlx/myNanoLog/testfile/spd-async.txt");

        auto spdlog_benchmark = [&spd_logger](int i, char const* const cstr) { spd_logger->info("Logging {}{}{}{}{}", cstr, i, 0, 'K', -42.42); };
        for (auto threads : {1, 2, 3, 4, 5})
            run_benchmark(spdlog_benchmark, threads, "spdlog");
    }else{
        print_usage();
        return 0;
    }
}