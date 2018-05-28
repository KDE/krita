/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016 Jeff Preshing
  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction
  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef MAPTRAITS_H
#define MAPTRAITS_H

#include <QtCore>

inline quint64 roundUpPowerOf2(quint64 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

inline bool isPowerOf2(quint64 v)
{
    return (v & (v - 1)) == 0;
}

template <class T>
struct DefaultKeyTraits {
    typedef T Key;
    typedef quint32 Hash;
    static const Key NullKey = Key(0);
    static const Hash NullHash = Hash(0);

    static Hash hash(T key)
    {
        return std::hash<Hash>()(Hash(key));
    }
};

template <class T>
struct DefaultValueTraits {
    typedef T Value;
    typedef quint32 IntType;
    static const IntType NullValue = 0;
    static const IntType Redirect = 1;
};

#endif // MAPTRAITS_H
