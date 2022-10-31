#include "myNanoLog"

int main() {
    // 初始化一个保障型日志
    std::string log_dir = "/home/wlx/myNanoLog/";                                            // 存储log文件的路径
    std::string file_name = "logging";                                                       // log文件名
    uint32_t file_size_mb = 1;                                                               // 每个log文件最大1mb
    myNanoLog::initialize(myNanoLog::GuaranteedLogger(), log_dir, file_name, file_size_mb);  // 初始化保障型日志器
    // nanolog::initialize(nanolog::NonGuaranteedLogger(3), "/home/wlx/myNanoLog/", "myNanoLog_ngl", 1); // 初始化非保障型日志器

    // 测试
    for (int i = 0; i < 200000; ++i) {
        LOG_DEBUG << "Sample NanoLog: " << i << " over";
    }

    // 可以实时修改日志等级，例如这里修改成CRITICAL等级
    myNanoLog::set_log_level(myNanoLog::LogLevel::CRITICAL);
    LOG_WARNING << "HAHAHAHAHA not printed!!!";

    return 0;
}