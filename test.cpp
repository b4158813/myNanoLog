#include "NanoLog.cpp"

int main() {

    // 初始化一个保障型日志
    myNanoLog::initialize(myNanoLog::GuaranteedLogger(), "/home/wlx/myNanoLog/", "myNanoLog_ngl", 1);
    // nanolog::initialize(nanolog::NonGuaranteedLogger(3), "/tmp/", "nanolog", 1);

    // 测试
    for (int i = 0; i < 15; ++i) {
        LOG_INFO << "Sample NanoLog: " << i << " over";
    }

    // 可以实时修改日志等级
    myNanoLog::set_log_level(myNanoLog::LogLevel::CRIT);
    LOG_WARN << "HAHAHAHAHA not printed!!!";

    return 0;
}