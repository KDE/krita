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

/**
 * Coverts a list of objects with type T into a list of objects of type R.
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

