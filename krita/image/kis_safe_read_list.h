/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_SAFE_READ_LIST_H_
#define KIS_SAFE_READ_LIST_H_


#include <QList>

/**
 * \class KisSafeReadList
 *
 * This is a special wrapper around QList class
 * Q: Why is it needed?
 * A: It guarantees thread-safety of all the read requests to the list.
 *    There is absolutely *no* guarantees for write requests though.
 * Q: Why pure QList cannot guarantee it?
 * A: First, Qt does not guarantee thread-safety for QList at all.
 *    Second, QList is implicitly shared structure, therefore even
 *    with read, but non-const requests (e.g. non-const QList::first()),
 *    QList will perform internal write operations. That will lead to
 *    a race condition in an environment with 3 and more threads.
 */
template<class T> class KisSafeReadList : private QList<T> {
public:
    KisSafeReadList() {}

    using typename QList<T>::const_iterator;

    /**
     * All the methods of this class are split into two groups:
     * treadsafe and non-threadsafe. The methods from the first group
     * can be called concurrently with each other. The ones form
     * the other group can't be called concurrently (even with the
     * firends from the first group) and must have an exclusive
     * access to the list.
     */

    /**
     * The thread-safe group
     */

    inline const T& first() const {
        return QList<T>::first();
    }

    inline const T& last() const {
        return QList<T>::last();
    }

    inline const T& at(int i) const {
        return QList<T>::at(i);
    }

    using QList<T>::constBegin;
    using QList<T>::constEnd;
    using QList<T>::isEmpty;
    using QList<T>::size;
    using QList<T>::indexOf;
    using QList<T>::contains;

    /**
     * The non-thread-safe group
     */

    using QList<T>::append;
    using QList<T>::prepend;
    using QList<T>::insert;
    using QList<T>::removeAt;
    using QList<T>::clear;

private:
    Q_DISABLE_COPY(KisSafeReadList);
};


#define FOREACH_SAFE(_iter, _container)         \
    for(_iter = _container.constBegin();        \
        _iter != _container.constEnd();         \
        _iter++)


#endif /* KIS_SAFE_READ_LIST_H_ */
