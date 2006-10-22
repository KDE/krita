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
        
template<class T>
class KisSharedPtr {
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
    
        /**
        * Copies a pointer.
        * @param o the pointer to copy
        */
        inline KisSharedPtr<T>( const KisSharedPtr<T>& o )
            : d(o.d) { if(d) d->ref.ref(); }
    
        
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
     * Note that if you use this function, the pointer might be destroyed if KisSharedPtr pointing
     * to this pointer are deleted, resulting in a segmentation fault. Use with care.
     * @return the pointer
     */
    inline T* data() { return d; }

    /**
     * Note that if you use this function, the pointer might be destroyed if KisSharedPtr pointing
     * to this pointer are deleted, resulting in a segmentation fault. Use with care.
     * @return the pointer
     */
    inline const T* data() const { return d; }

    /**
     * Attach the given pointer to the current KisSharedPtr.
     * If the previous shared pointer is not owned by any KisSharedPtr,
     * it is deleted.
     */
    void attach(T* p) const;

    /**
     * Clear the pointer, i.e. make it a null pointer.
     */
    void clear();

    /**
     * Detach the pointer by attaching a new copy of the pointer.
     * The new copy is created only if the pointer is shared by other pointers
     * and is not null.
     */
    void detach();

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
        
        /**
        * Unreferences the object that this pointer points to. If it was
        * the last reference, the object will be deleted.
        */
        inline ~KisSharedPtr() { if (d && !d->ref.deref()) delete d; }
    private:
        mutable T* d;
};

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

template <class T>
Q_INLINE_TEMPLATE void KisSharedPtr<T>::detach()
{
    if (d && d->ref>1) {
        attach(new T(*d));
    }
}

#endif
