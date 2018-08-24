/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_POINTER_UTILS_H
#define KIS_POINTER_UTILS_H

#include <QSharedPointer>

/**
 * Convert a raw pointer into a shared pointer
 */
template <class T>
inline QSharedPointer<T> toQShared(T* ptr) {
    return QSharedPointer<T>(ptr);
}

/**
 * Convert a list of raw pointers into a list of shared pointers
 */
template <class A, template <class C> class List>
List<QSharedPointer<A>> listToQShared(const List<A*> list) {
    List<QSharedPointer<A>> newList;
    Q_FOREACH(A* value, list) {
        newList.append(toQShared(value));
    }
    return newList;
}


/**
 * Convert a list of strong pointers into a list of weak pointers
 */
template <template <class> class Container, class T>
Container<QWeakPointer<T>> listStrongToWeak(const Container<QSharedPointer<T>> &containter)
{
    Container<QWeakPointer<T> > result;
    Q_FOREACH (QSharedPointer<T> v, containter) {
        result << v;
    }
    return result;
}

/**
 * Convert a list of weak pointers into a list of strong pointers
 *
 * WARNING: By default, uses "all or nothing" rule. If at least one of
 *          the weak pointers is invalid, returns an *empty* list!
 *          Even though some other pointer can still be converted
 *          correctly.
 */
template <template <class> class Container, class T>
    Container<QSharedPointer<T> > listWeakToStrong(const Container<QWeakPointer<T>> &containter,
                                                   bool allOrNothing = true)
{
    Container<QSharedPointer<T> > result;
    Q_FOREACH (QWeakPointer<T> v, containter) {
        QSharedPointer<T> strong(v);
        if (!strong && allOrNothing) {
            result.clear();
            return result;
        }

        if (strong) {
            result << strong;
        }
    }
    return result;
}

/**
 * Converts a list of objects with type T into a list of objects of type R.
 * The conversion is done implicitly, therefore the c-tor of type R should
 * support it. The main usage case is conversion of pointers in "descendant-
 * to-parent" way.
 */
template <typename R, typename T>
inline QList<R> implicitCastList(const QList<T> &list)
{
    QList<R> result;

    Q_FOREACH(const T &item, list) {
        result.append(item);
    }
    return result;
}

#endif // KIS_POINTER_UTILS_H

