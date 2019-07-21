/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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

#ifndef KIS_NEW_ON_COPY_H_
#define KIS_NEW_ON_COPY_H_

/**
 * This class wraps around some type T that is not copiable.
 * When the copy-constructor or assignment of KisNewOnCopy<T> is called,
 * it default-constructs an instance of T.
 */
template<typename T>
class KisNewOnCopy
{
public:
    KisNewOnCopy() : instance() {}
    KisNewOnCopy(const KisNewOnCopy &) : instance() {}

    // KisNewOnCopy &operator=(const KisNewOnCopy &) { return *this; }

    const T *data() const { return &instance; }
    const T *constData() { return &instance; }
    T *data() { return &instance; }
    const T *operator->() const { return &instance; }
    T *operator->() { return &instance; }

private:
    T instance;
};

#endif
