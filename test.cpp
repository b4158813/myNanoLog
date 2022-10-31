#include "NanoLog.cpp"

int main() {

    // 初始化一个保障型日志
    myNanoLog::initialize(myNanoLog::GuaranteedLogger(), "/home/wlx/myNanoLog/", "myNanoLog", 1);
    // nanolog::initialize(nanolog::NonGuaranteedLogger(3), "/home/wlx/myNanoLog/", "myNanoLog_ngl", 1);

    // 测试
    for (int i = 0; i < 15; ++i) {
        LOG_DEBUG << "Sample NanoLog: " << i << " over";
    }

    // 可以实时修改日志等级，例如这里修改成CRITICAL等级
    myNanoLog::set_log_level(myNanoLog::LogLevel::CRITICAL);
    LOG_WARNING << "HAHAHAHAHA not printed!!!";

    return 0;
}