/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISLAZYSHAREDCACHESTORAGE_H
#define KISLAZYSHAREDCACHESTORAGE_H

#include <type_traits>
#include <functional>
#include <utility>

#include <QMutex>
#include <QAtomicPointer>
#include <QSharedPointer>

template <typename T, typename... Args>
class KisLazySharedCacheStorage
{
public:
    using ConstType = std::add_const_t<T>;
    using FactoryType = T*(Args...);

public:
    KisLazySharedCacheStorage()
    {
    }

    KisLazySharedCacheStorage(std::function<FactoryType> factory)
        : m_factory(factory)
    {
    }

    KisLazySharedCacheStorage(const KisLazySharedCacheStorage &rhs) {
        QMutexLocker l(&rhs.m_mutex);
        m_factory = rhs.m_factory;
        m_value = rhs.m_value;
        m_cachedValue = rhs.m_cachedValue;
    }

    void setFactory(std::function<FactoryType> factory) {
        m_factory = factory;
    }

    ConstType* value(Args... args) {
        ConstType *result = 0;

        if (m_cachedValue) {
            result = m_cachedValue;
        } else {
            QMutexLocker l(&m_mutex);

            if (!m_value) {
                m_value.reset(m_factory(args...));
            }

            m_cachedValue = m_value.data();
            result = m_value.data();
        }
        return result;
    }

    bool isNull() const {
        return !bool(m_cachedValue);
    }

    void initialize(Args... args) {
        (void) value(args...);
    }

    void reset() {
        QMutexLocker l(&m_mutex);
        m_cachedValue.store(nullptr);
        m_value.reset();
    }

private:
    std::function<FactoryType> m_factory;
    QSharedPointer<ConstType> m_value;
    QAtomicPointer<ConstType> m_cachedValue;
    mutable QMutex m_mutex;
};

#endif // KISLAZYSHAREDCACHESTORAGE_H
