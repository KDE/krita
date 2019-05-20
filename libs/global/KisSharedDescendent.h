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

#ifndef KIS_SHARED_DESCENDENT_H_
#define KIS_SHARED_DESCENDENT_H_

#include "KisDescendent.h"

#include <QSharedData>

/**
 * To implement copy-on-write for your class:
 * (1) If it is a class without subclasses, or the subclasses do
 * not share a d-pointer with the base class:
 *
 * Make YourClass::Private inherit from QSharedData.
 * Replace `QScopedPointer<Private> d;` with `QSharedDataPointer<Private> d;`.
 *
 * (2) If it is a class with subclasses, and the subclasses share
 * a (inherited) d-pointer with the base class:
 *
 * Replace `QScopedPointer<BaseClassPrivate> d_ptr;` with
 *     `QSharedDataPointer<KisSharedDescendent<BaseClassPrivate> > d_ptr;` in your base class.
 * Remove all `Q_DECLARE_PRIVATE()`s.
 * Remove all `Q_DISABLE_COPY()`s. (To obtain copy-on-write you must first allow copies.)
 * Replace `explicit DerivedClass(BaseClassPrivate &dd) : BaseClass(dd) {}` with
 *     `explicit DerivedClass(KisSharedDescendent<BaseClassPrivate> &dd) : BaseClass(dd) {}`.
 * Replace `DerivedClass() : BaseClass(*(new DerivedClassPrivate())) {}` with
 *     `DerivedClass() : BaseClass(KisSharedDescendent<BaseClassPrivate>::of(DerivedClassPrivate())) {}`.
 * Replace all `Q_D()` macros that get non-const d-pointers with `SHARED_D(YourClass)`.
 * Replace all `Q_D()` macros that get const d-pointers with `CONST_SHARED_D(YourClass)`.
 */

#define CONST_SHARED_D(Class) const Class##Private *const d = reinterpret_cast<const Class##Private *>(d_ptr.constData()->constData())
#define SHARED_D(Class) Class##Private *const d = reinterpret_cast<Class##Private *>(d_ptr.data()->data())

template<typename T>
class KisSharedDescendent : public KisDescendent<T>, public QSharedData
{
public:
    template<typename U>
    KisSharedDescendent(U x) : KisDescendent<T>(std::move(x)) {}

    template<typename U>
    constexpr static KisSharedDescendent &of(U x) { return *(new KisSharedDescendent<T>(std::move(x))); }

    template<typename U>
    constexpr static KisSharedDescendent *pointerOf(U x) { return new KisSharedDescendent<T>(std::move(x)); }
};


#endif // KIS_SHARED_DESCENDENT_H_
