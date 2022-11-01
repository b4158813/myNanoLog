#include "FileWriter.h"

namespace myNanoLog {

// 控制数据写入文件的类
FileWriter::FileWriter(const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
    : m_log_file_roll_size_bytes(log_file_roll_size_mb * 1024 * 1024), m_name(log_directory + log_file_name) {
    roll_file();  // 初始化，新开第一个文件，准备写入
}

void FileWriter::write(myNanoLog::NanoLogLine& logline) {  // 写logline数据操作
    auto pos = m_os->tellp();                              // 得到当前将要写入的缓冲区的位置
    logline.stringify(*m_os);                              // 写入logline数据到文件中
    m_bytes_written += m_os->tellp() - pos;                // 获取刚才写入了多少字节数据
    if (m_bytes_written > m_log_file_roll_size_bytes) {
        roll_file();
    }
}

void FileWriter::roll_file() {  // 超过roll size了，需要新开一个文件写
    if (m_os) {
        m_os->flush();  // 刷新缓冲区，把未输出内容全部输出
        m_os->close();  // 切断当前文件输出流和其他文件的关系
    }

    m_bytes_written = 0;                                                   // 写入字节数清零
    m_os.reset(new std::ofstream());                                       // 重置输出流对象
    std::string log_file_name = m_name;                                    // 得到新的文件的名字
    log_file_name.append("_" + std::to_string(++m_file_number) + ".txt");  // 添加文件名后缀
    m_os->open(log_file_name, std::ofstream::out | std::ofstream::trunc);  // 将输出流重定向到新的文件
}

}  // namespace myNanoLog
