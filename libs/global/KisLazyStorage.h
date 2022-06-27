/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISLAZYSTORAGE_H
#define KISLAZYSTORAGE_H

#include <functional>
#include <memory>
#include <atomic>

#include <QMutex>
#include <QMutexLocker>
#include "KisMpl.h"

/**
 * KisLazyStorage is a special class implementing some kind
 * of lazy evaluation. It mimics the behaviorof a normal
 * pointer type, but creates `T` only on the first trt to
 * dereference this pointer.
 */
template <typename T, typename... Args>
class KisLazyStorage
{
public:
    struct init_value_tag { explicit init_value_tag() = default; };

    /**
     * Create a storage with a deferred creation of the object
     * `T`. The arguments \p args are passed to the constructor
     * of `T` on the first dereference operation on the storage.
     */
    explicit KisLazyStorage(Args... args)
        : m_constructionArgs(std::forward<Args>(args)...),
          m_data(0)
    {
    }

    /**
     * Create a storage and initialize it with \p value
     * right away without any deferring. Please take it into
     * account that the storage will still store a default-
     * initialized values of types `Args...` in an internal
     * tuple. If you want to avoid this default-construction,
     * then just wrap the arguments into std::optional.
     */
    explicit KisLazyStorage(init_value_tag, T &&value)
        : m_data(new T(std::forward<T>(value)))
    {
    }

    KisLazyStorage(const KisLazyStorage &rgh) = delete;
    KisLazyStorage(KisLazyStorage &&rgh) = delete;


    ~KisLazyStorage() {
        delete m_data.load();
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
                m_data = kismpl::apply_r<T*>(&constructObject, m_constructionArgs);
            }
        }
        return m_data;
    }

    static inline T* constructObject(Args... args) {
        return new T(std::forward<Args>(args)...);
    }

private:
    std::tuple<Args...> m_constructionArgs;
    std::atomic<T*> m_data;
    QMutex m_mutex;
};

#endif // KISLAZYSTORAGE_H
