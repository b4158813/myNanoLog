#include <bits/stdc++.h>
#include "myNanoLog"

// void fun(){ // test for fomatting time

//     std::time_t time_now_us = std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now().time_since_epoch())).count();
//     std::time_t time_now_s = time_now_us / 1000000;
//     time_now_s += 28800; // UTC + 8
//     std::tm* time_now = std::gmtime(&time_now_s);

//     char buff[32], usbuff[7];
//     strftime(buff, 32, "%Y-%m-%d %H:%M:%S.", time_now);
//     sprintf(usbuff, "%06llu", time_now_us % 1000000);
//     std::cout << buff << usbuff << std::endl;
// }

int main() {
    // 初始化一个保障型日志
    std::string log_dir = "/home/wlx/myNanoLog/";                                            // 存储log文件的路径
    std::string file_name = "logging";                                                       // log文件名
    uint32_t file_size_mb = 1;                                                               // 每个log文件最大1mb
    myNanoLog::initialize(myNanoLog::GuaranteedLogger(), log_dir, file_name, file_size_mb);  // 初始化保障型日志器
    // nanolog::initialize(nanolog::NonGuaranteedLogger(3), "/home/wlx/myNanoLog/", "myNanoLog_ngl", 1); // 初始化非保障型日志器

    // 测试
    for (int i = 0; i < 20; ++i) {
        LOG_DEBUG << "Sample NanoLog: " << i << " over";
    }

    // 可以实时修改日志等级，例如这里修改成CRITICAL等级
    myNanoLog::set_log_level(myNanoLog::LogLevel::CRITICAL);
    LOG_WARNING << "HAHAHAHAHA not printed!!!";

    return 0;
}