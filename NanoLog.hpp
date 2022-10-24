#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <type_traits>

namespace myNanoLog {

enum class LogLevel : uint8_t {
    INFO,
    WARN,
    CRIT
};

class NanoLogLine {
   public:
    // 构造
    NanoLogLine(LogLevel level, const char* file, const char* function, uint32_t line);
    // 析构
    ~NanoLogLine();
    // 移动构造
    NanoLogLine(NanoLogLine&&) = default;
    // 移动赋值运算符
    NanoLogLine& operator=(NanoLogLine&&) = default;
    // 重载输出流
    NanoLogLine& operator<<(char arg);
    NanoLogLine& operator<<(int32_t arg);
    NanoLogLine& operator<<(uint32_t arg);
    NanoLogLine& operator<<(int64_t arg);
    NanoLogLine& operator<<(uint64_t arg);
    NanoLogLine& operator<<(double arg);
    NanoLogLine& operator<<(std::string const& arg);

    // 针对c字符数组的偏特化
    template <size_t N>
    NanoLogLine& operator<<(const char (&arg)[N]) {
        encode(string_literal_t(arg));
        return *this;
    }

    // 针对c字符数组指针的偏特化
    template <typename T>
    typename std::enable_if<std::is_same<T, const char*>::value, NanoLogLine&>::type
    operator<<(const T& arg) {
        encode(arg);
        return *this;
    }

    // 同上
    template <typename T>
    typename std::enable_if<std::is_same<T, char*>::value, NanoLogLine&>::type
    operator<<(const T& arg) {
        encode(arg);
        return *this;
    }

    // 字符串字面量类型
    struct string_literal_t {
        explicit string_literal_t(const char* s)
            : m_s(s) {}
        const char* m_s;
    };

    void stringify(std::ostream& os);

   private:
    char* buffer();

    template <typename T>
    void encode(T arg);

    template <typename T>
    void encode(T arg, uint8_t type_id);

    void encode(char* arg);
    void encode(const char* arg);
    void encode(string_literal_t arg);
    void encode_c_string(const char* arg, size_t length);

    void resize_buffer_if_needed(size_t additional_bytes);
    void stringify(std::ostream& os, char * start, char const * const end);

   private:
    size_t m_bytes_used;                    // 已使用的字节数
    size_t m_buffer_size;                   // 缓冲区大小
    std::unique_ptr<char[]> m_heap_buffer;  // 堆内存区的缓冲区
    // 栈内存区的缓冲区
    char m_stack_buffer[256 - (sizeof(size_t) << 1) - sizeof(decltype(m_heap_buffer)) - 8];
};

struct NanoLog {
    bool operator==(NanoLogLine&);
};

// 设置日志等级
void set_log_level(LogLevel level);

// 是否记录
bool is_logged(LogLevel level);

struct NonGuaranteedLogger {
    NonGuaranteedLogger(uint32_t ring_buffer_size_mb_)
        : ring_buffer_size_mb(ring_buffer_size_mb_) {}
    uint32_t ring_buffer_size_mb;
};

struct GuaranteedLogger {
};

/* 初始化日志系统

*/
void initialize(GuaranteedLogger gl, const std::string& log_dir, const std::string& log_file_name, uint32_t log_file_maxsize_mb);

void initialize(NonGuaranteedLogger gl, const std::string& log_dir, const std::string& log_file_name, uint32_t log_file_maxsize_mb);

}  // namespace myNanoLog