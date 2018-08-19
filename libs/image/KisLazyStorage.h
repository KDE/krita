#ifndef KISLAZYSTORAGE_H
#define KISLAZYSTORAGE_H

#include <functional>
#include <memory>
#include <atomic>

#include <QMutex>
#include <QMutexLocker>


template <typename T>
class KisLazyStorage
{
public:
    template<typename P1>
    explicit KisLazyStorage(P1 p1)
        : m_builder([=]() { return new T(p1); }),
          m_data(0)
    {
    }

    template<typename P1, typename P2>
    explicit KisLazyStorage(P1 p1, P2 p2)
        : m_builder([=]() { return new T(p1, p2); }),
          m_data(0)
    {
    }

    template<typename P1, typename P2, typename P3>
    explicit KisLazyStorage(P1 p1, P2 p2, P2 p3)
        : m_builder([=]() { return new T(p1, p2, p3); }),
          m_data(0)
    {
    }

    KisLazyStorage(const KisLazyStorage &rgh) = delete;
    KisLazyStorage(KisLazyStorage &&rgh) = delete;


    ~KisLazyStorage() {
        delete m_data;
    }

    T* operator->() {
        return getPointer();
    }

    T& operator*() {
        return *getPointer();
    }

private:
    T* getPointer() {
        if(!m_data) {
            QMutexLocker l(&m_mutex);
            if(!m_data) {
                m_data = m_builder();
            }
        }
        return m_data;
    }

private:
    std::function<T*()> m_builder;
    std::atomic<T*> m_data;
    QMutex m_mutex;
};

#endif // KISLAZYSTORAGE_H
