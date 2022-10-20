#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>
#include <type_traits>

namespace NanoLog {

enum class LogLevel : uint8_t {
    INFO,
    WARN,
    CRIT
};

class NanoLogLine {
   public:
   	// 构造
    NanoLogLine(LogLevel level, const char* file, const char *function, uint32_t line);
	// 析构
	~NanoLogLine();
	// 移动构造
	NanoLogLine(NanoLogLine&&) = default;
	// 移动赋值运算符
	NanoLogLine& operator=(NanoLogLine&&) = default;
	NanoLogLine& operator << (char arg);
	NanoLogLine& operator << (int32_t arg);
	NanoLogLine& operator << (uint32_t arg);
	NanoLogLine& operator << (int64_t arg);
	NanoLogLine& operator << (uint64_t arg);
	NanoLogLine& operator << (double arg);
	NanoLogLine& operator << (std::string const &arg);
	
	template <size_t N>
	NanoLogLine& operator << (const char (&arg)[N]){
		encode(string_literal_t(arg));
		return *this;
	}

	
	// template <typename T>
	// typename std::enable_if<std::is_same<Arg, cosnt char *>::value
};

}  // namespace NanoLog