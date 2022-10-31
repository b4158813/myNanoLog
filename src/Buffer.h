#include <memory>
#include <queue>
#include "myNanoLog.h"
#include "SpinLock.hpp"

namespace myNanoLog {
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

    RingBuffer(const size_t size);
    ~RingBuffer();
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    void push(NanoLogLine&& logline) override;
    bool try_pop(NanoLogLine& logline) override;

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

    Buffer();
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    // 往buffer里加数据，返回buffer有没有满
    bool push(NanoLogLine&& logline, const uint32_t write_index);
    bool try_pop(NanoLogLine& logline, const uint32_t read_index);

   private:
    Item* m_buffer;
    std::atomic<uint32_t> m_write_state[size + 1];
};

// 基于队列的缓冲区
class QueueBuffer : public BufferBase {
   public:
    QueueBuffer(const QueueBuffer&) = delete;
    QueueBuffer& operator=(const QueueBuffer&) = delete;
    QueueBuffer();
    void push(NanoLogLine&& logline) override;
    bool try_pop(NanoLogLine& logline) override;

   private:
    void setup_next_write_buffer();
    Buffer* get_next_read_buffer();

   private:
    std::queue<std::unique_ptr<Buffer>> m_buffers;  // std::atomic不能用于实现std::queue的各种操作!!!
    std::atomic<Buffer*> m_current_write_buffer;
    Buffer* m_current_read_buffer;
    std::atomic<uint32_t> m_write_index;
    std::atomic_flag m_flag;
    uint32_t m_read_index;
};

}  // namespace myNanoLog
