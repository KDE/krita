/*
 * This file is part of the Krita project
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

/**
 * Gives a random access to the pixel of an image. Use the moveTo function, to select the pixel. And then rawData to
 * access the value of a pixel.
 */
class KisRandomAccessor{
    public:
        KisRandomAccessor(KisTiledDataManager *ktm, Q_INT32 x, Q_INT32 y, Q_INT32 offsetx, Q_INT32 offsety, bool writable);
        KisRandomAccessor(const KisRandomAccessor& rhs);
        ~KisRandomAccessor();
    public:
        /// Move to a given x,y position, fetch tiles and data
        void moveTo(Q_INT32 x, Q_INT32 y);
        /// @return a pointer to the pixel value
        Q_UINT8* rawData() const;
        /// @return a pointer to the old pixel value
        const Q_UINT8* oldRawData() const;
    private:
        KisTiledRandomAccessorSP m_accessor;
        Q_INT32 m_offsetx, m_offsety;
};

/**
 * This class provided access to information about the selection through the random accessor.
 */
class KisRandomAccessorPixelTrait {
    public:
        inline KisRandomAccessorPixelTrait(KisRandomAccessor* underlyingAccessor, KisRandomAccessor* selectionAccessor) : m_underlyingAccessor(underlyingAccessor), m_selectionAccessor(selectionAccessor)
        {
        }
        ~KisRandomAccessorPixelTrait() {
            if(m_selectionAccessor)
                delete m_selectionAccessor;
        }
        /// @return true if the pixel is selected
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

/**
 * Gives a random access to the pixel of an image. Use the moveTo function, to select the pixel. And then rawData to
 * access the value of a pixel.
 * The function isSelected() and selectedness() gives you access to the selection of the current pixel.
 * Note, that you should use this class only if you need random access to a pixel. It is best to use iterators as much as possible.
 */
class KisRandomAccessorPixel : public KisRandomAccessor, public KisRandomAccessorPixelTrait {
    public:
        KisRandomAccessorPixel(KisTiledDataManager *ktm, KisTiledDataManager *ktmselect, Q_INT32 x, Q_INT32 y, Q_INT32 offsetx, Q_INT32 offsety, bool writable);
    public:
        inline void moveTo(Q_INT32 x, Q_INT32 y) { KisRandomAccessor::moveTo(x,y); KisRandomAccessorPixelTrait::moveTo(x,y); }
};


#endif
