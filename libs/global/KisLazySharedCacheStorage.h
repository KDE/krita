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
#include <QSharedDataPointer>
#include <QExplicitlySharedDataPointer>

namespace KisLazySharedCacheStorageDetail
{

/**
 * A policy-like class for sharing the **data** between two
 * objects
 */
template <typename StorageType, typename T, typename... Args>
struct DataStorage
{
    using ConstType = std::add_const_t<T>;
    using FactoryType = T*(Args...);

    DataStorage() {}
    DataStorage(T *value) : m_value(value) {}
    DataStorage(const DataStorage &rhs) = default;
    DataStorage& operator=(const DataStorage &rhs) = default;

    ConstType* lazyInitialize(const std::function<FactoryType> &factory, Args... args) {
        if (!m_value) {
            m_value.reset(factory(std::forward<Args...>(args...)));
        }
        return m_value.data();
    }

    bool hasValue() const {
        return !m_value.isNull();
    }

    void reset() {
        m_value.reset();
    }

private:
    StorageType m_value;
};

template <typename T, typename... Args>
using DataWrapperLocal = DataStorage<QSharedPointer<T>, T, Args...>;


/**
 * A policy-like class for sharing DataStorage object
 * between the two different objects. It just ads one
 * more level of locking to ensure thread-safety.
 */
template <typename T, typename... Args>
struct DataWrapperShared
{
    using ConstType = std::add_const_t<T>;
    using FactoryType = T*(Args...);

private:
    struct SharedStorage
    {
        using ConstType = std::add_const_t<T>;

        SharedStorage() {}
        SharedStorage(T *value) : m_value(value) {}
        SharedStorage(const SharedStorage &rhs) = delete;

        QMutex sharedMutex;
        DataStorage<QScopedPointer<ConstType>, T, Args...> m_value;
    };
public:


    DataWrapperShared() : m_sharedStorage(new SharedStorage()) {}
    DataWrapperShared(T *value) : m_sharedStorage(new SharedStorage(value)) {}
    DataWrapperShared(const DataWrapperShared &rhs) = default;
    DataWrapperShared& operator=(const DataWrapperShared &rhs) = default;

    ConstType* lazyInitialize(const std::function<FactoryType> &factory, Args... args) {
        QMutexLocker l(&m_sharedStorage->sharedMutex);
        return m_sharedStorage->m_value.lazyInitialize(factory, std::forward<Args...>(args...));
    }

    bool hasValue() const {
        QMutexLocker l(&m_sharedStorage->sharedMutex);
        return m_sharedStorage->m_value.hasValue();
    }

    void reset() {
        m_sharedStorage.reset(new SharedStorage());
    }

private:
    QSharedPointer<SharedStorage> m_sharedStorage;
};

}

template <typename DataWrapper, typename T, typename... Args>
class KisLazySharedCacheStorageBase
{
public:
    using ConstType = std::add_const_t<T>;
    using FactoryType = T*(Args...);

public:
    KisLazySharedCacheStorageBase()
    {
    }

    KisLazySharedCacheStorageBase(std::function<FactoryType> factory)
        : m_factory(factory)
    {
    }

    KisLazySharedCacheStorageBase(const KisLazySharedCacheStorageBase &rhs) {
        QMutexLocker l(&rhs.m_mutex);
        m_factory = rhs.m_factory;
        m_dataWrapper = rhs.m_dataWrapper;
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
            QMutexLocker l1(&m_mutex);
            m_cachedValue = m_dataWrapper.lazyInitialize(m_factory, args...);
            result = m_cachedValue;
        }
        return result;
    }

    bool isNull() const {
        return !bool(m_cachedValue) && !m_dataWrapper.hasValue();
    }

    void initialize(Args... args) {
        (void) value(args...);
    }

    void reset() {
        QMutexLocker l(&m_mutex);
        m_cachedValue.store(nullptr);
        m_dataWrapper.reset();
    }

private:
    std::function<FactoryType> m_factory;
    DataWrapper m_dataWrapper;
    QAtomicPointer<ConstType> m_cachedValue;
    mutable QMutex m_mutex;
};

/**
 * KisLazySharedCacheStorage is a special class that allows two classes
 * lazily share an existing cache data. That is, after the class has been
 * cloned via a copy constructor it keeps a pointer to the cached data of
 * the source object. This link is kept until one of the objects calls
 * `cache.reset()`, then the cached data is detached.
 *
 * Take it into account that KisLazySharedCacheStorage will keep a link
 * **only** in case the cache existed at the moment of cloning. That is
 * the main difference to KisLazySharedCacheStorageLinked.
 *
 * The class guarantees full tread-safety of the access and detach of
 * the cache object.
 */
template <typename T, typename... Args>
using KisLazySharedCacheStorage =
    KisLazySharedCacheStorageBase<
        KisLazySharedCacheStorageDetail::DataWrapperLocal<T, Args...>, T, Args...>;

/**
 * KisLazySharedCacheStorageLinked is a special class that allows two classes
 * lazily share a cache object. The two classes will not only share cache data,
 * but also the cache object itself. That is, when one of the objects updates
 * the cache data, this data will also be uploaded into the cache of the other
 * object. The link between the two objects is kept until one of the objects
 * calls `cache.reset()`.
 *
 * The class guarantees full tread-safety of the access and detach of
 * the cache object.
 */
template <typename T, typename... Args>
using KisLazySharedCacheStorageLinked =
    KisLazySharedCacheStorageBase<
        KisLazySharedCacheStorageDetail::DataWrapperShared<T, Args...>, T, Args...>;


#endif // KISLAZYSHAREDCACHESTORAGE_H
