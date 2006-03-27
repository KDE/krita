/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_SHARED_PTR_VECTOR_H
#define KIS_SHARED_PTR_VECTOR_H

#include <q3valuevector.h>

#include <ksharedptr.h>

/**
 * QValueVector does not always destroy an element when it is erased.
 * If the items it is holding are KSharedPtr, this can result in hidden
 * references to objects that cannot be accounted for. This class
 * sets the KSharedPtr to 0 on erase, which dereferences the object as
 * expected.
 */
template <class T>
class KisSharedPtrVector : public Q3ValueVector< KSharedPtr<T> >
{
    typedef Q3ValueVector< KSharedPtr<T> > super;
public:
    KisSharedPtrVector() {}

    void pop_back()
    {
            if (!super::empty()) {
            super::back() = 0;
            super::pop_back();
        }
    }

    typename super::iterator erase(typename super::iterator it)
    {
        *it = 0;
        return super::erase(it);
    }

    typename super::iterator erase(typename super::iterator first, typename super::iterator last)
    {
        qFill(first, last, 0);
        return super::erase(first, last);
    }

    bool contains(KSharedPtr<T> ptr) const
    {
        for (int i = 0, n = super::count(); i < n; ++i)
            if (super::at(i) == ptr)
                return true;
        return false;
    }
};

#endif // KIS_SHARED_PTR_VECTOR_H

