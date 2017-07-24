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

#ifndef KRITA_CONTAINER_UTILS_H
#define KRITA_CONTAINER_UTILS_H

#include <functional>

namespace KritaUtils
{

template <class T>
    bool compareListsUnordered(const QList<T> &a, const QList<T> &b) {
    if (a.size() != b.size()) return false;

    Q_FOREACH(const T &t, a) {
        if (!b.contains(t)) return false;
    }

    return true;
}

template <class C>
    void makeContainerUnique(C &container) {
    std::sort(container.begin(), container.end());
    auto newEnd = std::unique(container.begin(), container.end());

    while (newEnd != container.end()) {
        newEnd = container.erase(newEnd);
    }
}


template <class C, typename KeepIfFunction>
    auto filterContainer(C &container, KeepIfFunction keepIf)
        -> decltype(bool(keepIf(container[0])), void()) {

        auto newEnd = std::remove_if(container.begin(), container.end(), [keepIf] (typename C::reference p) { return !keepIf(p); });
        while (newEnd != container.end()) {
           newEnd = container.erase(newEnd);
        }
}

}


#endif // KRITA_CONTAINER_UTILS_H

