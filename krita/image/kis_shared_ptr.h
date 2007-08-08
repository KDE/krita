/*
 *  Copyright (c) 2005 Frerich Raabe <raabe@kde.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_SHAREDPTR_H
#define KIS_SHAREDPTR_H

#include <qglobal.h>

#include <kis_shared_data.h>

template<class T>
class KisWeakSharedPtr;

/**
 * KisSharedPtr is a shared pointer similar to KSharedPtr and
 * boost::shared_ptr. The difference with KSharedPtr is that our
 * constructor is not explicit. The difference with boost is still to
 * be determined.
 *
 * A shared pointer is a wrapper around a real pointer. The shared
 * pointer keeps a reference count, and when the refernece count drops
 * to 0 the contained pointer is deleted. You can use the shared
 * pointer just as you would use a real pointer.
 *
 * See also also item 28 and 29 of More Effective C++ and
 * http://bugs.kde.org/show_bug.cgi?id=52261 as well as
 * http://www.boost.org/libs/smart_ptr/shared_ptr.htm.
 *
 * XXX: We should replace all uses of KisSharedPtr with the boost
 * shared pointer, if possible.
 */
template<class T>
class KisSharedPtr {
    friend class KisWeakSharedPtr<T>;
    public:
        /**
         * Creates a null pointer.
         */
        inline KisSharedPtr()
            : d(0) { }

        /**
         * Creates a new pointer.
         * @param p the pointer
         */
        inline KisSharedPtr( T* p )
            : d(p) { if(d) d->ref.ref(); }

        inline KisSharedPtr(const KisWeakSharedPtr<T>& o);

        /**
         * Copies a pointer.
         * @param o the pointer to copy
         */
        inline KisSharedPtr<T>( const KisSharedPtr<T>& o )
            : d(o.d) { if(d) d->ref.ref(); }

        /**
         * Unreferences the object that this pointer points to. If it was
         * the last reference, the object will be deleted.
         */
        inline ~KisSharedPtr() { if (d && !d->ref.deref()) { delete d; } }

        inline KisSharedPtr<T>& operator= ( const KisSharedPtr& o ) { attach(o.d); return *this; }
        inline const KisSharedPtr<T>& operator= ( const KisSharedPtr& o ) const {
            attach(o.d);
            return *this;
        }
        inline bool operator== ( const T* p ) const { return ( d == p ); }
        inline bool operator!= ( const T* p ) const { return ( d != p ); }
        inline bool operator== ( const KisSharedPtr& o ) const { return ( d == o.d ); }
        inline bool operator!= ( const KisSharedPtr& o ) const { return ( d != o.d ); }

        inline KisSharedPtr<T>& operator= ( T* p ) { attach(p); return *this; }

        inline operator const T* () const { return d; }

        template< class T2> inline operator KisSharedPtr<T2>() const { return KisSharedPtr<T2>(d); }

        /**
        * @return the contained pointer. If you delete the contained
        * pointer, you will make KisSharedPtr very unhappy. If you put
        * the contained pointer in another KisSharedPtr, they won't
        * know about each other, which again, will make one of the
        * shared pointers very unhappy at some time.
        */
        inline T* data() { return d; }

        /**
        * @return the pointer
        */
        inline const T* data() const { return d; }

        /**
        * it is deleted.
        */
        void attach(T* p) const;

        /**
        * Clear the pointer, i.e. make it a null pointer.
        */
        void clear();

        /**
        * @return a const pointer to the shared object.
        */
        inline const T* constData() const { return d; }

        inline const T& operator*() const { Q_ASSERT(d); return *d; }
        inline T& operator*() { Q_ASSERT(d); return *d; }
        inline const T* operator->() const { Q_ASSERT(d); return d; }
        inline T* operator->() { Q_ASSERT(d); return d; }

        /**
        * @return true if the pointer is null
        */
        inline bool isNull() const { return (d == 0); }

    private:
        mutable T* d;
};

/**
 * A weak shared ptr is an ordinary shared ptr, with one difference:
 * it doesn't delete the contained pointer if the refcount drops to
 * zero.
 */
template<class T>
class KisWeakSharedPtr {
    friend class KisSharedPtr<T>;
    public:
        /**
         * Creates a null pointer.
         */
        inline KisWeakSharedPtr()
            : d(0) { }

        /**
         * Creates a new pointer.
         * @param p the pointer
         */
        inline KisWeakSharedPtr( T* p )
            : d(p)
        {
            if(d) dataPtr = d->dataPtr;
        }

        inline KisWeakSharedPtr<T>( const KisSharedPtr<T>& o )
            : d(o.d) {
            if(d) dataPtr = d->dataPtr;
        }
        /**
         * Copies a pointer.
         * @param o the pointer to copy
         */
        inline KisWeakSharedPtr<T>( const KisWeakSharedPtr<T>& o )
            : d(o.d) { if(d) dataPtr = d->dataPtr; }

        inline KisWeakSharedPtr<T>& operator= ( const KisWeakSharedPtr& o )
        {
            attach(o.d); return *this;
        }
        inline const KisWeakSharedPtr<T>& operator= ( const KisWeakSharedPtr& o ) const {
            attach(o.d);
            return *this;
        }
        inline bool operator== ( const T* p ) const { return ( d == p ); }
        inline bool operator!= ( const T* p ) const { return ( d != p ); }
        inline bool operator== ( const KisWeakSharedPtr& o ) const { return ( d == o.d ); }
        inline bool operator!= ( const KisWeakSharedPtr& o ) const { return ( d != o.d ); }

        inline KisWeakSharedPtr<T>& operator= ( T* p ) {
            attach(p);
            return *this;
        }

        inline operator const T* () const { Q_ASSERT(not d or( dataPtr and dataPtr->valid)); return d; }

        template< class T2> inline operator KisWeakSharedPtr<T2>() const { return KisWeakSharedPtr<T2>(d); }

        /**
        * Note that if you use this function, the pointer might be destroyed if KisSharedPtr pointing
        * to this pointer are deleted, resulting in a segmentation fault. Use with care.
        * @return the pointer
        */
        inline T* data() {
            Q_ASSERT(not d or( dataPtr and dataPtr->valid));
            return d;
        }

        /**
        * Note that if you use this function, the pointer might be destroyed if KisSharedPtr pointing
        * to this pointer are deleted, resulting in a segmentation fault. Use with care.
        * @return the pointer
        */
        inline const T* data() const {
            Q_ASSERT(not d or( dataPtr and dataPtr->valid));
            return d;
        }

        /**
        * Note that if you use this function, the pointer might be destroyed if KisSharedPtr pointing
        * to this pointer are deleted, resulting in a segmentation fault. Use with care.
        * @return a const pointer to the shared object.
        */
        inline const T* constData() const { Q_ASSERT(not d or( dataPtr and dataPtr->valid)); return d; }

        inline const T& operator*() const { Q_ASSERT(d and dataPtr and dataPtr->valid); return *d; }
        inline T& operator*() { Q_ASSERT(d and dataPtr and dataPtr->valid); return *d; }
        inline const T* operator->() const { Q_ASSERT(d and dataPtr and dataPtr->valid); return d; }
        inline T* operator->() { Q_ASSERT(d and dataPtr and dataPtr->valid); return d; }

        /**
        * @return true if the pointer is null
        */
        inline bool isNull() const { return (d == 0); }

        /**
         * @return true if the weak pointer points to a valid pointer and false if
         *         the data has been deleted or is null
         */
        inline bool isValid() const { return d and dataPtr and dataPtr->valid; }
    private:
        void attach(T* nd)
        {
            d = nd;
            if(d) dataPtr = d->dataPtr;
            else dataPtr = 0;
        }
        mutable T* d;
        KisSharedPtr< KisSharedData > dataPtr;
};


template <class T>
Q_INLINE_TEMPLATE  KisSharedPtr<T>::KisSharedPtr(const KisWeakSharedPtr<T>& o)
 : d(o.d)
{
    Q_ASSERT(not o.dataPtr or ( o.dataPtr and o.dataPtr->valid));
    if(d) d->ref.ref();
}


template <class T>
Q_INLINE_TEMPLATE void KisSharedPtr<T>::attach(T* p) const
{
    if (d != p) {
        T *x = p;
        if (x) x->ref.ref();
        x = qAtomicSetPtr(&d, x);
        if (x && !x->ref.deref())
            delete x;
    }
}

template <class T>
Q_INLINE_TEMPLATE void KisSharedPtr<T>::clear()
{
    attach((T*)0);
}

#endif
