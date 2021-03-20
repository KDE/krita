/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
