/*
 * This file is part of the Krita project
 *
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

#ifndef KIS_RANDOM_ACCESSOR_H
#define KIS_RANDOM_ACCESSOR_H

#include <ksharedptr.h>

#include <kis_global.h>

class KisTiledRandomAccessor;
typedef KSharedPtr<KisTiledRandomAccessor> KisTiledRandomAccessorSP;

class KisTiledDataManager;

class KisRandomAccessor{
    public:
        KisRandomAccessor(KisTiledDataManager *ktm, Q_INT32 x, Q_INT32 y, Q_INT32 offsetx, Q_INT32 offsety, bool writable);
        KisRandomAccessor(const KisRandomAccessor& rhs);
        ~KisRandomAccessor();
    public:
        /// Move to a given x,y position, fetch tiles and data
        void moveTo(Q_INT32 x, Q_INT32 y);
        Q_UINT8* rawData() const;
        const Q_UINT8* oldRawData() const;
    private:
        KisTiledRandomAccessorSP m_accessor;
        Q_INT32 m_offsetx, m_offsety;
};

class KisRandomAccessorPixelTrait {
    public:
        inline KisRandomAccessorPixelTrait(KisRandomAccessor* underlyingAccessor, KisRandomAccessor* selectionAccessor) : m_underlyingAccessor(underlyingAccessor), m_selectionAccessor(selectionAccessor)
        {
        }
	~KisRandomAccessorPixelTrait() {
		if(m_selectionAccessor)
			delete m_selectionAccessor;
	}
        inline bool isSelected() const
        {
            return (m_selectionAccessor) ? *(m_selectionAccessor->rawData()) > SELECTION_THRESHOLD : true;
        };
        inline Q_UINT8 operator[](int index) const
        { return m_underlyingAccessor->rawData()[index]; };
        /**
         * Returns the degree of selectedness of the pixel.
         */
        inline Q_UINT8 selectedness() const
        {
            return (m_selectionAccessor) ? *(m_selectionAccessor->rawData()) : MAX_SELECTED;
        };

        /**
         * Returns the selectionmask from the current point; this is guaranteed
         * to have the same number of consecutive pixels that the iterator has
         * at a given point. It return a 0 if there is no selection.
         */
        inline Q_UINT8 * selectionMask() const
        {
            return ( m_selectionAccessor ) ? m_selectionAccessor->rawData() : 0;
        }

        inline void moveTo(Q_INT32 x, Q_INT32 y) { if(m_selectionAccessor) m_selectionAccessor->moveTo(x,y); }
        
    private:
        KisRandomAccessor* m_underlyingAccessor;
        KisRandomAccessor* m_selectionAccessor;
};

class KisRandomAccessorPixel : public KisRandomAccessor, public KisRandomAccessorPixelTrait {
    public:
        KisRandomAccessorPixel(KisTiledDataManager *ktm, KisTiledDataManager *ktmselect, Q_INT32 x, Q_INT32 y, Q_INT32 offsetx, Q_INT32 offsety, bool writable);
    public:
        inline void moveTo(Q_INT32 x, Q_INT32 y) { KisRandomAccessor::moveTo(x,y); KisRandomAccessorPixelTrait::moveTo(x,y); }
};


#endif
