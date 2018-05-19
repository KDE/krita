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

#include "Util.h"

template <class T>
struct DefaultKeyTraits {
    typedef T Key;
    typedef typename BestFit<T>::Unsigned Hash;
    static const Key NullKey = Key(0);
    static const Hash NullHash = Hash(0);

    static Hash hash(T key)
    {
        return avalanche(Hash(key));
    }

    static Key dehash(Hash hash)
    {
        return (T) deavalanche(hash);
    }
};

template <class T>
struct DefaultValueTraits {
    typedef T Value;
    typedef typename BestFit<T>::Unsigned IntType;
    static const IntType NullValue = 0;
    static const IntType Redirect = 1;
};

#endif // MAPTRAITS_H
