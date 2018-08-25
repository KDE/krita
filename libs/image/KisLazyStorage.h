/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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
