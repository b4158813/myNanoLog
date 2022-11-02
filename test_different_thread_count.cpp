#include <bits/stdc++.h>
#include "myNanoLog"
constexpr int maxtid = 5;


std::ofstream out;

// 获取当前时间，精确到ns
uint64_t timestamp_now() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

void fun_benchmark() {
    const int maxiter = 1E5;
    const char* str = "benchmark";
    uint64_t st = timestamp_now();
    for (int i = 0; i < maxiter; ++i) {
        LOG_INFO << "test" << ':' << str << 123 << -456.789;
    }
    uint64_t en = timestamp_now();
    out << "Average latency for each iteration: " << (en - st) / maxiter << " ns\n";
}

template <typename F>
void test_diff_thread(F&& f, int thread_count) {
    out << "-----Thread Count: " << thread_count << "-----" << std::endl;
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(f);
    }
    for (int i = 0; i < thread_count; ++i) {
        threads[i].join();
    }
}

void test_diff_thread_NGL() {
    out.open("/home/wlx/myNanoLog/testfile/test_NGL.md");
    myNanoLog::initialize(myNanoLog::NonGuaranteedLogger(10), "/home/wlx/myNanoLog/testfile/", "test_NGL", 1);
    out << "Test NonGuaranteedLogger for different thread counts\n\n";
    for (int tcnt = 1; tcnt <= maxtid; ++tcnt) {
        test_diff_thread(fun_benchmark, tcnt);
    }
}

void test_diff_thread_GL(){
    out.open("/home/wlx/myNanoLog/testfile/test_GL.md");
    myNanoLog::initialize(myNanoLog::GuaranteedLogger(), "/home/wlx/myNanoLog/testfile/", "test_GL", 1);
    out << "Test GuaranteedLogger for different thread counts\n\n";
    for (int tcnt = 1; tcnt <= maxtid; ++tcnt) {
        test_diff_thread(fun_benchmark, tcnt);
    }
}

void print_usage() {
    char const* const executable = "test_threadcount";
    printf("Usage \n1. %s GL   (for GuaranteedLogger)\n2. %s NGL   (for NonGuaranteedLogger)\n", executable, executable);
}


int main(int argc, char **argv) {
    if(argc != 2){
        print_usage();
        return 0;
    }
    if(strcmp(argv[1], "NGL") == 0){
        test_diff_thread_NGL();
    }else if(strcmp(argv[1], "GL") == 0){
        test_diff_thread_GL();
    }else{
        print_usage();
        return 0;
    }

    return 0;
}