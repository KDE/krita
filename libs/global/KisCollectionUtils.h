/*
 *  Copyright (c) 2019 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef KRITA_KISCOLLECTIONUTILS_H
#define KRITA_KISCOLLECTIONUTILS_H

namespace KisCollectionUtils {
    /**
     * Finds the last item the in the map with a key less than or equal to the given one.
     * Returns map.constEnd() if no such key exists.
     */
    template<class T>
    typename QMap<int, T>::const_iterator lastBeforeOrAt(const QMap<int, T> &map, int maximumKey)
    {
        typename QMap<int, T>::const_iterator i = map.upperBound(maximumKey);
        if (i == map.constBegin()) return map.constEnd();
        return i - 1;
    }

    /**
     * Finds the last item the in the map with a key strictly less than the given key.
     * Returns map.constEnd() if no such key exists.
     */
    template<class T>
    typename QMap<int, T>::const_iterator lastBefore(const QMap<int, T> &map, int currentKey)
    {
        typename QMap<int, T>::const_iterator active = lastBeforeOrAt(map, currentKey);
        if (active == map.constEnd()) return map.constEnd();

        if (currentKey > active.key()) return active;

        if (active == map.constBegin()) return map.constEnd();
        return active - 1;
    }

    /**
     * Finds the first item the in the map with a key greater than the given one.
     * Returns map.constEnd() if no such key exists.
     */
    template<class T>
    typename QMap<int, T>::const_iterator firstAfter(const QMap<int, T> &map, int currentKey)
    {
        return map.upperBound(currentKey);
    }
}

#endif //KRITA_KISCOLLECTIONUTILS_H
