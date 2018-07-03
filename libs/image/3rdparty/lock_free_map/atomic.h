/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016 Jeff Preshing
  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction
  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef ATOMIC_H
#define ATOMIC_H

#include <atomic>

inline void signalFenceConsume()
{
    std::atomic_signal_fence(std::memory_order_acquire);
}

inline void signalFenceAcquire()
{
    std::atomic_signal_fence(std::memory_order_acquire);
}

inline void signalFenceRelease()
{
    std::atomic_signal_fence(std::memory_order_release);
}

inline void signalFenceSeqCst()
{
    std::atomic_signal_fence(std::memory_order_seq_cst);
}

inline void threadFenceConsume()
{
    std::atomic_thread_fence(std::memory_order_acquire);
}

inline void threadFenceAcquire()
{
    std::atomic_thread_fence(std::memory_order_acquire);
}

inline void threadFenceRelease()
{
    std::atomic_thread_fence(std::memory_order_release);
}

inline void threadFenceSeqCst()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

enum MemoryOrder {
    Relaxed = std::memory_order_relaxed,
    Consume = std::memory_order_consume,
    Acquire = std::memory_order_acquire,
    Release = std::memory_order_release,
    ConsumeRelease = std::memory_order_acq_rel,
    AcquireRelease = std::memory_order_acq_rel,
};

template <typename T>
class Atomic : protected std::atomic<T>
{
private:
    T operator=(T value);

public:
    Atomic()
    {
    }

    Atomic(T value) : std::atomic<T>(value)
    {
    }

    Atomic(const Atomic& other) : std::atomic<T>(other.loadNonatomic())
    {
    }

    T loadNonatomic() const
    {
        return std::atomic<T>::load(std::memory_order_relaxed);
    }

    T load(MemoryOrder memoryOrder) const
    {
        return std::atomic<T>::load((std::memory_order) memoryOrder);
    }

    void storeNonatomic(T value)
    {
        return std::atomic<T>::store(value, std::memory_order_relaxed);
    }

    void store(T value, MemoryOrder memoryOrder)
    {
        return std::atomic<T>::store(value, (std::memory_order) memoryOrder);
    }

    T compareExchange(T expected, T desired, MemoryOrder memoryOrder)
    {
        std::atomic<T>::compare_exchange_strong(expected, desired, (std::memory_order) memoryOrder);
        return expected; // modified by reference by compare_exchange_strong
    }

    bool compareExchangeStrong(T& expected, T desired, MemoryOrder memoryOrder)
    {
        return std::atomic<T>::compare_exchange_strong(expected, desired, (std::memory_order) memoryOrder);
    }

    bool compareExchangeWeak(T& expected, T desired, MemoryOrder success, MemoryOrder failure)
    {
        return std::atomic<T>::compare_exchange_weak(expected, desired, (std::memory_order) success, (std::memory_order) failure);
    }

    T exchange(T desired, MemoryOrder memoryOrder)
    {
        return std::atomic<T>::exchange(desired, (std::memory_order) memoryOrder);
    }

    T fetchAdd(T operand, MemoryOrder memoryOrder)
    {
        return std::atomic<T>::fetch_add(operand, (std::memory_order) memoryOrder);
    }

    T fetchSub(T operand, MemoryOrder memoryOrder)
    {
        return std::atomic<T>::fetch_sub(operand, (std::memory_order) memoryOrder);
    }

    T fetchAnd(T operand, MemoryOrder memoryOrder)
    {
        return std::atomic<T>::fetch_and(operand, (std::memory_order) memoryOrder);
    }

    T fetchOr(T operand, MemoryOrder memoryOrder)
    {
        return std::atomic<T>::fetch_or(operand, (std::memory_order) memoryOrder);
    }
};

#endif // ATOMIC_H
