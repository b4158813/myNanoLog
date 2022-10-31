#include<atomic>

// 封装的自旋锁类(RAII)
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