#include <fstream>
#include <memory>
#include <string>

namespace myNanoLog {
// 控制数据写入文件的类
class FileWriter {
   public:
    FileWriter(const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb);

    void write(myNanoLog::NanoLogLine& logline);

   private:
    void roll_file();

   private:
    uint32_t m_file_number = 0;                 // 当前正在写的文件的编号
    std::streamoff m_bytes_written = 0;         // 已经在当前文件写了多少字节的数据
    uint32_t const m_log_file_roll_size_bytes;  // 自定义的文件大小阈值
    std::string const m_name;                   // 需要输出的文件的名字（绝对路径）
    std::unique_ptr<std::ofstream> m_os;
};
}  // namespace myNanoLog
