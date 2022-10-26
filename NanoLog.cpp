#include "NanoLog.hpp"
#include <atomic>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <queue>
#include <thread>
#include <tuple>

namespace {
// 获取精确到μs的时间
uint64_t timestamp_now() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

// 格式化时间
void format_timestamp(std::ostream& os, uint64_t timestamp) {
    std::time_t time_t = timestamp / 1000000;
    time_t += 28800;  // UTC + 8 转成东八区中国时间！
    auto gmtime = std::gmtime(&time_t);
    char buff[32], microseconds[7];
    strftime(buff, 32, "%Y-%m-%d %H:%M:%S.", gmtime);
    sprintf(microseconds, "%06llu", timestamp % 1000000);
    os << '[' << buff << microseconds << ']';
}

// 获取当前线程tid
std::thread::id this_thread_id() {
    static thread_local const std::thread::id id = std::this_thread::get_id();
    return id;
}

template <typename T, typename Tuple>
struct TupleIndex;

template <typename T, typename... Types>
struct TupleIndex<T, std::tuple<T, Types...>> {
    static constexpr const std::size_t value = 0;
};

template <typename T, typename U, typename... Types>
struct TupleIndex<T, std::tuple<U, Types...>> {
    static constexpr const std::size_t value = 1 + TupleIndex<T, std::tuple<Types...>>::value;
};

}  // namespace

namespace myNanoLog {

using SupportedTypes = std::tuple<char, uint32_t, uint64_t, int32_t, int64_t, double, NanoLogLine::string_literal_t, char*>;

const char* to_string(LogLevel loglevel) {
    switch (loglevel) {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::CRIT:
            return "CRIT";
    }
    return "NONE";
}

// NanoLogLine 构造函数
NanoLogLine::NanoLogLine(LogLevel level, const char* file, const char* function, uint32_t line)
    : m_bytes_used(0), m_buffer_size(sizeof(m_stack_buffer)) {
    encode<uint64_t>(timestamp_now());
    encode<std::thread::id>(this_thread_id());
    encode<string_literal_t>(string_literal_t(file));
    encode<string_literal_t>(string_literal_t(function));
    encode<uint32_t>(line);
    encode<LogLevel>(level);
}
// NanoLogLine 析构函数
NanoLogLine::~NanoLogLine() = default;

template <typename Arg>
void NanoLogLine::encode(Arg arg) {
    *reinterpret_cast<Arg*>(buffer()) = arg;
    m_bytes_used += sizeof(Arg);
}

template <typename Arg>
void NanoLogLine::encode(Arg arg, uint8_t type_id) {
    resize_buffer_if_needed(sizeof(Arg) + sizeof(uint8_t));
    encode<uint8_t>(type_id);
    encode<Arg>(arg);
}

// 根据dummy类型输出指定类型的内容
template <typename Arg>
char* decode(std::ostream& os, char* b, Arg* dummy) {
    Arg arg = *reinterpret_cast<Arg*>(b);
    os << arg;
    return b + sizeof(Arg);
}
// 针对string_literal_t的偏特化
template <>
char* decode(std::ostream& os, char* b, NanoLogLine::string_literal_t* dummy) {
    NanoLogLine::string_literal_t s = *reinterpret_cast<NanoLogLine::string_literal_t*>(b);
    os << s.m_s;
    return b + sizeof(NanoLogLine::string_literal_t);
}
// 针对原生字符指针的偏特化
template <>
char* decode(std::ostream& os, char* b, char** dummy) {
    while (*b != '\0') {
        os << *b;
        ++b;
    }
    return ++b;
}
// 将日志内容格式化并输出（前缀 + 文本）
void NanoLogLine::stringify(std::ostream& os) {
    char* b = (m_heap_buffer == nullptr ? m_stack_buffer : m_heap_buffer.get());
    char const* const end = b + m_bytes_used;
    uint64_t timestamp = *reinterpret_cast<uint64_t*>(b);
    b += sizeof(uint64_t);
    std::thread::id tid = *reinterpret_cast<std::thread::id*>(b);
    b += sizeof(std::thread::id);
    string_literal_t file = *reinterpret_cast<string_literal_t*>(b);
    b += sizeof(string_literal_t);
    string_literal_t function = *reinterpret_cast<string_literal_t*>(b);
    b += sizeof(string_literal_t);
    uint32_t line = *reinterpret_cast<uint32_t*>(b);
    b += sizeof(uint32_t);
    LogLevel loglevel = *reinterpret_cast<LogLevel*>(b);
    b += sizeof(LogLevel);

    format_timestamp(os, timestamp);

    // 输出前缀内容
    os << '[' << to_string(loglevel) << ']'
       << '[' << tid << ']'
       << '[' << file.m_s << ':' << function.m_s << ':' << line << ']'
       << ' ';

    // 输出剩余内容
    stringify(os, b, end);
    if (loglevel >= LogLevel::CRIT) {
        os.flush();
    }
}

// 将日志内容格式化并输出（文本）
void NanoLogLine::stringify(std::ostream& os, char* start, char const* const end) {
    if (start == end) {
        return;
    }

    int type_id = static_cast<int>(*start);
    start++;
    switch (type_id) {
        case 0:
            stringify(os, decode(os, start, static_cast<std::tuple_element<0, SupportedTypes>::type*>(nullptr)), end);
            return;
        case 1:
            stringify(os, decode(os, start, static_cast<std::tuple_element<1, SupportedTypes>::type*>(nullptr)), end);
            return;
        case 2:
            stringify(os, decode(os, start, static_cast<std::tuple_element<2, SupportedTypes>::type*>(nullptr)), end);
            return;
        case 3:
            stringify(os, decode(os, start, static_cast<std::tuple_element<3, SupportedTypes>::type*>(nullptr)), end);
            return;
        case 4:
            stringify(os, decode(os, start, static_cast<std::tuple_element<4, SupportedTypes>::type*>(nullptr)), end);
            return;
        case 5:
            stringify(os, decode(os, start, static_cast<std::tuple_element<5, SupportedTypes>::type*>(nullptr)), end);
            return;
        case 6:
            stringify(os, decode(os, start, static_cast<std::tuple_element<6, SupportedTypes>::type*>(nullptr)), end);
            return;
        case 7:
            stringify(os, decode(os, start, static_cast<std::tuple_element<7, SupportedTypes>::type*>(nullptr)), end);
            return;
    }
}

// 返回对应缓冲区（栈or堆）的尾部指针
char* NanoLogLine::buffer() {
    return (m_heap_buffer == nullptr ? &m_stack_buffer[m_bytes_used] : &(m_heap_buffer.get()[m_bytes_used]));
}

// 根据写入数据的大小自动调整（堆or栈）缓冲区大小
void NanoLogLine::resize_buffer_if_needed(size_t additional_bytes) {
    size_t const required_size = m_bytes_used + additional_bytes;

    if (required_size <= m_buffer_size) {
        return;
    }

    if (m_heap_buffer == nullptr) {
        m_buffer_size = std::max(static_cast<size_t>(512), required_size);
        m_heap_buffer.reset(new char[m_buffer_size]);
        memcpy(m_heap_buffer.get(), m_stack_buffer, m_bytes_used);
    } else {
        m_buffer_size = std::max(static_cast<size_t>(2 * m_buffer_size), required_size);
        std::unique_ptr<char[]> new_heap_buffer(new char[m_buffer_size]);
        memcpy(new_heap_buffer.get(), m_heap_buffer.get(), m_bytes_used);
        m_heap_buffer.swap(new_heap_buffer);
    }
}

// 针对原生C字符串
void NanoLogLine::encode(const char* arg) {
    if (arg != nullptr) {
        encode_c_string(arg, strlen(arg));
    }
}
void NanoLogLine::encode(char* arg) {
    if (arg != nullptr) {
        encode_c_string(arg, strlen(arg));
    }
}
void NanoLogLine::encode_c_string(const char* arg, size_t length) {
    if (length == 0) {
        return;
    }

    resize_buffer_if_needed(1 + length + 1);
    char* b = buffer();
    auto type_id = TupleIndex<char*, SupportedTypes>::value;
    *reinterpret_cast<uint8_t*>(b++) = static_cast<uint8_t>(type_id);
    memcpy(b, arg, 1 + length + 1);
    m_bytes_used += 1 + length + 1;
}

// 针对string_literal_t
void NanoLogLine::encode(string_literal_t arg) {
    encode<string_literal_t>(arg, TupleIndex<string_literal_t, SupportedTypes>::value);
}

// 针对每个不同类型，定义输出流
NanoLogLine& NanoLogLine::operator<<(std::string const& arg) {
    encode_c_string(arg.c_str(), arg.length());
    return *this;
}

NanoLogLine& NanoLogLine::operator<<(int32_t arg) {
    encode<int32_t>(arg, TupleIndex<int32_t, SupportedTypes>::value);
    return *this;
}

NanoLogLine& NanoLogLine::operator<<(uint32_t arg) {
    encode<uint32_t>(arg, TupleIndex<uint32_t, SupportedTypes>::value);
    return *this;
}

NanoLogLine& NanoLogLine::operator<<(int64_t arg) {
    encode<int64_t>(arg, TupleIndex<int64_t, SupportedTypes>::value);
    return *this;
}

NanoLogLine& NanoLogLine::operator<<(uint64_t arg) {
    encode<uint64_t>(arg, TupleIndex<uint64_t, SupportedTypes>::value);
    return *this;
}

NanoLogLine& NanoLogLine::operator<<(double arg) {
    encode<double>(arg, TupleIndex<double, SupportedTypes>::value);
    return *this;
}

NanoLogLine& NanoLogLine::operator<<(char arg) {
    encode<char>(arg, TupleIndex<char, SupportedTypes>::value);
    return *this;
}

// 封装的自旋锁类
struct SpinLock {
    SpinLock(std::atomic_flag& flag)
        : m_flag(flag) {
        while (m_flag.test_and_set(std::memory_order_acquire)) {
        }  // 拿锁
    }

    ~SpinLock() {
        m_flag.clear(std::memory_order_release);  // 释放锁
    }

   private:
    std::atomic_flag& m_flag;
};

struct BufferBase {
    virtual ~BufferBase() = default;
    virtual void push(NanoLogLine&& logline) = 0;
    virtual bool try_pop(NanoLogLine& logline) = 0;
};

// 多生产者单消费者 环形缓冲区
class RingBuffer : public BufferBase {
   public:
    struct alignas(64) Item {  // 内存对齐，对齐到64位
        Item()
            : flag(ATOMIC_FLAG_INIT), written(0), logline(LogLevel::INFO, nullptr, nullptr, 0) {}
        std::atomic_flag flag;
        char written;
        char padding[256 - sizeof(std::atomic_flag) - sizeof(char) - sizeof(NanoLogLine)];
        NanoLogLine logline;
    };

    RingBuffer(const size_t size)
        : m_size(size), m_ring(static_cast<Item*>(std::malloc(size * sizeof(Item)))), m_write_index(0), m_read_index(0) {
        for (size_t i = 0; i < m_size; ++i) {
            new (&m_ring[i]) Item();
        }
        static_assert(sizeof(Item) == 256, "Unexpected size != 256");
    }
    ~RingBuffer() {
        for (size_t i = 0; i < m_size; ++i) {
            m_ring[i].~Item();
        }
        std::free(m_ring);
    }
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    void push(NanoLogLine&& logline) override {
        uint32_t write_index = m_write_index.fetch_add(1, std::memory_order_relaxed) % m_size;
        Item& item = m_ring[write_index];
        SpinLock spinlock(item.flag);  // 拿锁
        item.logline = std::move(logline);
        item.written = 1;
        // 自动释放锁
    }
    bool try_pop(NanoLogLine& logline) {
        Item& item = m_ring[m_read_index % m_size];
        SpinLock spinlock(item.flag);
        if (item.written == 1) {
            logline = std::move(item.logline);
            item.written = 0;
            ++m_read_index;
            return true;
        }
        return false;
    }

   private:
    const size_t m_size;
    Item* m_ring;
    std::atomic<uint32_t> m_write_index;
    char pad[64];
    uint32_t m_read_index;
};

// 普通缓冲区
class Buffer {
   public:
    struct Item {
        Item(NanoLogLine&& nanologline)
            : logline(std::move(nanologline)) {}
        char padding[256 - sizeof(NanoLogLine)];
        NanoLogLine logline;
    };

    static constexpr const size_t size = 32768;

    Buffer()
        : m_buffer(static_cast<Item*>(std::malloc(size * sizeof(Item)))) {
        for (size_t i = 0; i <= size; ++i) {
            m_write_state[i].store(0, std::memory_order_relaxed);
        }
        static_assert(sizeof(Item) == 256, "Unexpectec size != 256");
    }
    ~Buffer() {
        uint32_t write_count = m_write_state[size].load();
        for (size_t i = 0; i < write_count; ++i) {
            m_buffer[i].~Item();
        }
        std::free(m_buffer);
    }
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    bool push(NanoLogLine&& logline, const uint32_t write_index) {
        new (&m_buffer[write_index]) Item(std::move(logline));
        m_write_state[write_index].store(1, std::memory_order_release);
        return m_write_state[size].fetch_add(1, std::memory_order_acquire);
    }
    bool try_pop(NanoLogLine& logline, const uint32_t read_index) {
        if (m_write_state[read_index].load(std::memory_order_acquire)) {
            Item& item = m_buffer[read_index];
            logline = std::move(item.logline);
            return true;
        }
        return false;
    }

   private:
    Item* m_buffer;
    std::atomic<uint32_t> m_write_state[size + 1];
};

// 基于队列的缓冲区
class QueueBuffer : public BufferBase {
   public:
    QueueBuffer(const QueueBuffer&) = delete;
    QueueBuffer& operator=(const QueueBuffer&) = delete;

    QueueBuffer()
        : m_current_read_buffer(nullptr), m_write_index(0), m_flag(ATOMIC_FLAG_INIT), m_read_index(0) {
        steup_next_write_buffer();
    }

    void push(NanoLogLine&& logline) override {
        uint32_t write_index = m_write_index.fetch_add(1, std::memory_order_relaxed);
        if (write_index < Buffer::size) {
            if (m_current_write_buffer.load(std::memory_order_acquire)->push(std::move(logline), write_index)) {
                steup_next_write_buffer();
            }
        } else {
            while (m_write_index.load(std::memory_order_acquire) >= Buffer::size) {
            }
            push(std::move(logline));
        }
    }

    bool try_pop(NanoLogLine& logline) override {
        if (m_current_read_buffer == nullptr) {
            m_current_read_buffer = get_next_read_buffer();
        }
        Buffer* read_buffer = m_current_read_buffer;

        if (read_buffer == nullptr) {
            return false;
        }
        if (bool success = read_buffer->try_pop(logline, m_read_index)) {
            m_read_index++;
            if (m_read_index == Buffer::size) {
                m_read_index = 0;
                m_current_read_buffer = nullptr;
                SpinLock spinlock(m_flag);
                m_buffers.pop();
            }
            return true;
        }
        return false;
    }

   private:
    void steup_next_write_buffer() {
        std::unique_ptr<Buffer> next_write_buffer(new Buffer());
        m_current_write_buffer.store(next_write_buffer.get(), std::memory_order_release);
        SpinLock spinlock(m_flag);
        m_buffers.push(std::move(next_write_buffer));
        m_write_index.store(0, std::memory_order_relaxed);
    }

    Buffer* get_next_read_buffer() {
        SpinLock spinlock(m_flag);
        return m_buffers.empty() ? nullptr : m_buffers.front().get();
    }

   private:
    std::queue<std::unique_ptr<Buffer>> m_buffers;
    std::atomic<Buffer*> m_current_write_buffer;
    Buffer* m_current_read_buffer;
    std::atomic<uint32_t> m_write_index;
    std::atomic_flag m_flag;
    uint32_t m_read_index;
};

// 控制数据写入文件的类
class FileWriter {
   public:
    FileWriter(const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
        : m_log_file_roll_size_bytes(log_file_roll_size_mb * 1024 * 1024), m_name(log_directory + log_file_name) {
        roll_file();  // 初始化，新开第一个文件，准备写入
    }

    void write(NanoLogLine& logline) {           // 写logline数据操作
        auto pos = m_os->tellp();                // 得到当前将要写入的缓冲区的位置
        logline.stringify(*m_os);                // 写入logline数据
        m_bytes_written += m_os->tellp() - pos;  // 获取刚才写入了多少字节数据
        if (m_bytes_written > m_log_file_roll_size_bytes) {
            roll_file();
        }
    }

   private:
    void roll_file() {  // 超过roll size了，需要新开一个文件写
        if (m_os) {
            m_os->flush();  // 刷新缓冲区，把未输出内容全部输出
            m_os->close();  // 切断当前文件输出流和其他文件的关系
        }

        m_bytes_written = 0;                                                   // 写入字节数清零
        m_os.reset(new std::ofstream());                                       // 重置输出流对象
        std::string log_file_name = m_name;                                    // 得到新的文件的名字
        log_file_name.append("." + std::to_string(++m_file_number) + ".txt");  // 添加文件名后缀
        m_os->open(log_file_name, std::ofstream::out | std::ofstream::trunc);  // 将输出流重定向到新的文件
    }

   private:
    uint32_t m_file_number = 0;          // 写入的文件数量
    std::streamoff m_bytes_written = 0;  // 已经在当前文件写了多少字节的数据
    uint32_t const m_log_file_roll_size_bytes;
    std::string const m_name;  // 需要输出的文件的名字（绝对路径）
    std::unique_ptr<std::ofstream> m_os;
};

class NanoLogger {  // NanoLogger主类
   public:
    // 非保障型的，基于环形缓冲区（在高强度写入的情况下，之前未写入的数据可能丢失，但不会阻止写入，即 新的会覆盖旧的）
    // 优点：充分利用空间，减少内存碎片产生
    // 缺点：在密集型写入任务中，可能造成数据丢失
    NanoLogger(NonGuaranteedLogger ngl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
        : m_state(State::INIT), m_buffer_base(new RingBuffer(std::max(1u, ngl.ring_buffer_size_mb) * 1024 * 4)), m_file_writer(log_directory, log_file_name, std::max(1u, log_file_roll_size_mb)) {
        m_thread = std::thread([this] { this->pop(); });
        m_state.store(State::READY, std::memory_order_release);
    }

    // 保障型的，基于队列的缓冲区，一定不会造成数据丢失
    // 优点：保障数据安全性，一定可以输出到文件中，维护方便，代码简单
    NanoLogger(GuaranteedLogger gl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb)
        : m_state(State::INIT), m_buffer_base(new QueueBuffer()), m_file_writer(log_directory, log_file_name, std::max(1u, log_file_roll_size_mb)) {
        m_thread = std::thread([this] { this->pop(); });
        m_state.store(State::READY, std::memory_order_release);
    }

    // 析构
    ~NanoLogger() {
        m_state.store(State::SHUTDOWN);  // state设置为关闭
        m_thread.join();                 // 回收线程资源
    }

    // 往缓冲区加数据，待写
    void add(NanoLogLine&& logline) {
        // 多态，根据BufferBase的派生类调用对应push函数
        m_buffer_base->push(std::move(logline));
    }

    void pop() {
        // 如果处于初始化状态，则循环等待直到状态为READY
        while (m_state.load(std::memory_order_acquire) == State::INIT) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }

        // 初始化一个空的NanoLogLine对象
        NanoLogLine logline(LogLevel::INFO, nullptr, nullptr, 0);

        // 如果NanoLogger是就绪状态，就循环尝试写数据到文件中
        while (m_state.load() == State::READY) {
            // 如果对应缓冲区能够pop出一行数据进行写，则调用FileWriter的写入操作
            if (m_buffer_base->try_pop(logline)) {
                m_file_writer.write(logline);
            } else {
                // 否则，就一直等到直到缓冲区能pop出一行进行写
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        }

        // shutdown之后，可能缓冲区还剩有数据，因此
        // 把缓冲区剩余内容pop出来写入文件
        while (m_buffer_base->try_pop(logline)) {
            m_file_writer.write(logline);
        }
    }

   private:
    enum class State {
        INIT,
        READY,
        SHUTDOWN
    };

    std::atomic<State> m_state;                 // 表示当前日志器的状态
    std::unique_ptr<BufferBase> m_buffer_base;  // 采用的缓冲区（多态）
    FileWriter m_file_writer;                   // FileWriter的一个实例对象，用于将数据写入文件
    std::thread m_thread;                       // 表示当前线程
};

std::unique_ptr<NanoLogger> nanologger;
std::atomic<NanoLogger*> atomic_nanologger;

bool NanoLog::operator==(NanoLogLine& logline) {
    atomic_nanologger.load(std::memory_order_acquire)->add(std::move(logline));
    return true;
}

void initialize(NonGuaranteedLogger ngl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb) {
    nanologger.reset(new NanoLogger(ngl, log_directory, log_file_name, log_file_roll_size_mb));
    atomic_nanologger.store(nanologger.get(), std::memory_order_seq_cst);
}

void initialize(GuaranteedLogger gl, const std::string& log_directory, const std::string& log_file_name, uint32_t log_file_roll_size_mb) {
    nanologger.reset(new NanoLogger(gl, log_directory, log_file_name, log_file_roll_size_mb));
    atomic_nanologger.store(nanologger.get(), std::memory_order_seq_cst);
}

std::atomic<uint32_t> loglevel{0};

void set_log_level(LogLevel level) {
    loglevel.store(static_cast<uint32_t>(level), std::memory_order_release);
}

bool is_logged(LogLevel level) {
    return static_cast<uint32_t>(level) >= loglevel.load(std::memory_order_relaxed);
}

}  // namespace myNanoLog