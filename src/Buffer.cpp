#include "Buffer.h"

namespace myNanoLog {
/*---------------------------------------------------------------------------------------*/
// 多生产者单消费者 环形缓冲区
RingBuffer::RingBuffer(const size_t size)
    : m_size(size), m_ring(static_cast<Item*>(std::malloc(size * sizeof(Item)))), m_write_index(0), m_read_index(0) {
    for (size_t i = 0; i < m_size; ++i) {
        new (&m_ring[i]) Item();
    }
    static_assert(sizeof(Item) == 256, "Unexpected size != 256");
}
RingBuffer::~RingBuffer() {
    for (size_t i = 0; i < m_size; ++i) {
        m_ring[i].~Item();
    }
    std::free(m_ring);
}

void RingBuffer::push(NanoLogLine&& logline) {
    uint32_t write_index = m_write_index.fetch_add(1, std::memory_order_relaxed) % m_size;
    Item& item = m_ring[write_index];
    SpinLock spinlock(item.flag);  // （自旋锁）拿锁
    item.logline = std::move(logline);
    item.written = 1;
    // 自动释放锁
}
bool RingBuffer::try_pop(NanoLogLine& logline) {
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

/*---------------------------------------------------------------------------------------*/
// 普通缓冲区(用于QueueBuffer)
Buffer::Buffer()
    : m_buffer(static_cast<Item*>(std::malloc(size * sizeof(Item)))) {
    for (size_t i = 0; i <= size; ++i) { // m_write_state[size] 用于记录改缓冲区对象有多少元素被使用了
        m_write_state[i].store(0, std::memory_order_relaxed);
    }
    static_assert(sizeof(Item) == 256, "Unexpected size != 256");
}
Buffer::~Buffer() {
    uint32_t write_count = m_write_state[size].load();
    for (size_t i = 0; i < write_count; ++i) {
        m_buffer[i].~Item();
    }
    std::free(m_buffer);
}
// 往buffer里加数据，返回buffer有没有满
bool Buffer::push(NanoLogLine&& logline, const uint32_t write_index) {
    new (&m_buffer[write_index]) Item(std::move(logline)); // placement new 初始化对象
    m_write_state[write_index].store(1, std::memory_order_release);
    return m_write_state[size].fetch_add(1, std::memory_order_acquire) + 1 == size;
}
bool Buffer::try_pop(NanoLogLine& logline, const uint32_t read_index) {
    if (m_write_state[read_index].load(std::memory_order_acquire)) {
        Item& item = m_buffer[read_index];
        logline = std::move(item.logline);
        return true;
    }
    return false;
}
/*---------------------------------------------------------------------------------------*/
// 基于队列的缓冲区
QueueBuffer::QueueBuffer()
    : m_current_read_buffer(nullptr), m_write_index(0), m_flag(ATOMIC_FLAG_INIT), m_read_index(0) {
    setup_next_write_buffer();
}

void QueueBuffer::push(NanoLogLine&& logline) {
    uint32_t write_index = m_write_index.fetch_add(1, std::memory_order_relaxed);
    if (write_index < Buffer::size) {
        // 如果当前Buffer用满了
        if (m_current_write_buffer.load(std::memory_order_acquire)->push(std::move(logline), write_index)) {
            // 开辟一个新的Buffer，并放入队列中，之后就用新的Buffer执行push
            setup_next_write_buffer();
        }
    } else {
        // 如果并发量过高，导致write_index超过了Buffer::size，则循环等待
        while (m_write_index.load(std::memory_order_acquire) >= Buffer::size) {}
        // 跳出循环后，说明write_index < Buffer::size，则再次尝试push
        push(std::move(logline));
    }
}

// 尝试将缓冲区日志内容pop到logline中
bool QueueBuffer::try_pop(NanoLogLine& logline) {
    if (m_current_read_buffer == nullptr) {
        m_current_read_buffer = get_next_read_buffer();
    }
    Buffer* read_buffer = m_current_read_buffer;

    if (read_buffer == nullptr) {
        return false;
    }
    if (read_buffer->try_pop(logline, m_read_index)) {
        m_read_index++;
        if (m_read_index == Buffer::size) { // 读到该Buffer最后一个Item了
            m_read_index = 0;
            m_current_read_buffer = nullptr;
            SpinLock spinlock(m_flag);
            m_buffers.pop();
        }
        return true;
    }
    return false;
}

// new一个新的Buffer指针，push进队列里（注意加锁）
void QueueBuffer::setup_next_write_buffer() {
    std::unique_ptr<Buffer> next_write_buffer(new Buffer());
    m_current_write_buffer.store(next_write_buffer.get(), std::memory_order_release);
    SpinLock spinlock(m_flag);
    m_buffers.push(std::move(next_write_buffer));
    m_write_index.store(0, std::memory_order_relaxed);
}
// 从队首获取下一个read_buffer
Buffer* QueueBuffer::get_next_read_buffer() {
    SpinLock spinlock(m_flag);
    return m_buffers.empty() ? nullptr : m_buffers.front().get();
}

}  // namespace myNanoLog
