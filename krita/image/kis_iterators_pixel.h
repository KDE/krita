/* This file is part of the KDE project
 *  Copyright (c) 2004-2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_ITERATORS_PIXEL_H_
#define KIS_ITERATORS_PIXEL_H_

#include "kis_iterator.h"
#include "kis_iteratorpixeltrait.h"

template<class T, typename TSelect>
class KisLineIteratorPixelBase : public T, public KisIteratorPixelTrait<T, TSelect>
{
    template<class T2, typename TSelect2> friend class KisLineIteratorPixelBase;
    public:
        KisLineIteratorPixelBase( KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 s, qint32 offsetx, qint32 offsety) :
            T(dm, x - offsetx, y - offsety, s),
        KisIteratorPixelTrait <T, TSelect> ( this ),
            m_offsetx(offsetx), m_offsety(offsety)
        {
            if(sel_dm) {
                T * i = new T(sel_dm, x - offsetx, y - offsety, s);
                Q_CHECK_PTR(i);
                KisIteratorPixelTrait <T, TSelect>::setSelectionIterator(i);
            }
        }
        template<class T2, typename TSelect2>
        KisLineIteratorPixelBase(const KisLineIteratorPixelBase<T2,TSelect2>& rhs) :
                T(rhs), KisIteratorPixelTrait <T, TSelect> (this)
        {
            if(rhs.selectionIterator())
            {
                KisIteratorPixelTrait <T, TSelect>::setSelectionIterator(new T(*rhs.selectionIterator()));
            }
            m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety;
        }
        /// increment the position of the iterator
        inline KisLineIteratorPixelBase<T, TSelect> & operator ++() { T::operator++(); KisIteratorPixelTrait<T, TSelect>::advance(1); return *this;}

        /// Advances a number of pixels until it reaches the end of the line
        KisLineIteratorPixelBase<T, TSelect> & operator+=(int n) { T::operator+=(n); KisIteratorPixelTrait<T, TSelect>::advance(n); return *this; };
        /// @return the x coordinate in the image referential
        qint32 x() const { return T::x() + m_offsetx; }
        /// @return the y coordinate in the image referential
        qint32 y() const { return T::y() + m_offsety; }
        /// @return the minimum of the regular underlying iterator's and the selection iterator's nConseqHPixels
        qint32 nConseqHPixels() const {
            if (this->m_selectionIterator) {
                qint32 parent = T::nConseqHPixels();
                qint32 selection = this->m_selectionIterator->nConseqHPixels();
                if (parent < selection)
                    return parent;
                return selection;
            }
            return T::nConseqHPixels();
        }
    private:
        qint32 m_offsetx, m_offsety;
};

template<class T, typename TSelect>
class KisRectIteratorPixelBase : public T, public KisIteratorPixelTrait<T, TSelect>
{
    template<class T2, typename TSelect2> friend class KisRectIteratorPixelBase;
    public:
        KisRectIteratorPixelBase( KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 w, qint32 h, qint32 offsetx, qint32 offsety) :
            T(dm, x - offsetx, y - offsety, w, h),
        KisIteratorPixelTrait <T, TSelect> ( this ),
        m_offsetx(offsetx), m_offsety(offsety)
        {
            if(sel_dm) {
                T * i = new T(sel_dm, x - offsetx, y - offsety, w, h);
                Q_CHECK_PTR(i);
                KisIteratorPixelTrait <T, TSelect>::setSelectionIterator(i);
            }
        }
        template<class T2, typename TSelect2>
        KisRectIteratorPixelBase(const KisRectIteratorPixelBase<T2, TSelect2>& rhs) :
                T(rhs), KisIteratorPixelTrait <T, TSelect> (this)
        {
            if(rhs.selectionIterator())
            {
                KisIteratorPixelTrait <T, TSelect>::setSelectionIterator(new T(*rhs.selectionIterator()));
            }
            m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety;
        }
        /// increment the position of the iterator
        inline KisRectIteratorPixelBase<T, TSelect> & operator ++() { T::operator++(); KisIteratorPixelTrait<T, TSelect>::advance(1); return *this;}

        /// Advances a number of pixels until it reaches the end of the line
        KisRectIteratorPixelBase<T, TSelect> & operator+=(int n) { T::operator+=(n); KisIteratorPixelTrait<T, TSelect>::advance(n); return *this; };
        /// @return the x coordinate in the image referential
        qint32 x() const { return T::x() + m_offsetx; }
        /// @return the y coordinate in the image referential
        qint32 y() const { return T::y() + m_offsety; }
        /// @return the minimum of the regular underlying iterator's and the selection iterator's nConseqHPixels
        qint32 nConseqPixels() const {
            if (this->m_selectionIterator) {
                qint32 parent = T::nConseqPixels();
                qint32 selection = this->m_selectionIterator->nConseqPixels();
                if (parent < selection)
                    return parent;
                return selection;
            }
            return T::nConseqPixels();
        }
    private:
        qint32 m_offsetx, m_offsety;
};

#endif
