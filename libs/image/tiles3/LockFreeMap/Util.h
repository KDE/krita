/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016 Jeff Preshing
  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction
  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

template <typename T>
struct BestFit;

template <>
struct BestFit<qint32> {
    typedef quint32 Unsigned;
    typedef qint32 Signed;
};

template <>
struct BestFit<quint32> {
    typedef quint32 Unsigned;
    typedef qint32 Signed;
};

template <>
struct BestFit<qint64> {
    typedef quint64 Unsigned;
    typedef qint64 Signed;
};

template <>
struct BestFit<quint64> {
    typedef quint64 Unsigned;
    typedef qint64 Signed;
};

template <class T>
struct BestFit<T*> {
    typedef quint64 Unsigned;
    typedef qint64 Signed;
};

inline quint32 roundUpPowerOf2(quint32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

inline qint32 roundUpPowerOf2(qint32 v)
{
    return (qint32) roundUpPowerOf2((quint32) v);
}

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

inline qint64 roundUpPowerOf2(qint64 v)
{
    return (qint64) roundUpPowerOf2((quint64) v);
}

inline bool isPowerOf2(quint64 v)
{
    return (v & (v - 1)) == 0;
}

// from code.google.com/p/smhasher/wiki/MurmurHash3
inline quint32 avalanche(quint32 h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

inline quint32 deavalanche(quint32 h)
{
    h ^= h >> 16;
    h *= 0x7ed1b41d;
    h ^= (h ^ (h >> 13)) >> 13;
    h *= 0xa5cb9243;
    h ^= h >> 16;
    return h;
}

// from code.google.com/p/smhasher/wiki/MurmurHash3
inline quint64 avalanche(quint64 h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}

inline quint64 deavalanche(quint64 h)
{
    h ^= h >> 33;
    h *= 0x9cb4b2f8129337db;
    h ^= h >> 33;
    h *= 0x4f74430c22a54005;
    h ^= h >> 33;
    return h;
}

#endif // UTIL_H
