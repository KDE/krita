/* This file is part of the KDE project
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>, the original iteratorpixel
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>, made it into a trait
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

#ifndef KIS_ITERATORPIXELTRAIT_H_
#define KIS_ITERATORPIXELTRAIT_H_

#include "kis_iterator.h"
#include <kis_paint_device.h>

template< typename _iTp>
class KisIteratorPixelTrait
{
public:
    KisIteratorPixelTrait(KisPaintDevice * ndevice, _iTp *underlyingIterator)
    :    m_device(ndevice),
        m_underlyingIterator(underlyingIterator)
    {
        m_selectionIterator = NULL;
    };

    ~KisIteratorPixelTrait()
    {
        delete m_selectionIterator;
    };

    KisIteratorPixelTrait(const KisIteratorPixelTrait& rhs)
    {
        if (this == &rhs)
            return;
        m_device = rhs.m_device;
        m_underlyingIterator = rhs.m_underlyingIterator;

        if (rhs.m_selectionIterator) {
            m_selectionIterator = new _iTp(*rhs.m_selectionIterator);
        } else {
            m_selectionIterator = 0;
        }
    }

    KisIteratorPixelTrait& operator=(const KisIteratorPixelTrait& rhs)
    {
        if (this == &rhs)
            return *this;
        m_device = rhs.m_device;
        m_underlyingIterator = rhs.m_underlyingIterator;

        delete m_selectionIterator;
        if (rhs.m_selectionIterator) {
            m_selectionIterator = new _iTp(*rhs.m_selectionIterator);
        } else {
            m_selectionIterator = 0;
        }

        return *this;
    }


public:
    /**
     * Return one channel from the current kispixel. Does not check whether
     * channel index actually exists in this colorspace.
     */
    inline quint8 operator[](int index) const
            { return m_underlyingIterator->rawData()[index]; };

    /**
     * Returns if the pixel is selected or not. This is much faster than first building a KisPixel
     */
    inline bool isSelected() const
        {
            if (m_selectionIterator)
                return *(m_selectionIterator->rawData()) > SELECTION_THRESHOLD;
            else
                return true;
        };

    /**
      * Returns the degree of selectedness of the pixel.
      */
    inline quint8 selectedness() const
        {
            if (m_selectionIterator)
                return *(m_selectionIterator->rawData());
            else {
                return MAX_SELECTED;
            }
        };

    /**
     * Returns the selectionmask from the current point; this is guaranteed
     * to have the same number of consecutive pixels that the iterator has
     * at a given point. It return a 0 if there is no selection.
     */
    inline quint8 * selectionMask() const
        {
            if ( m_selectionIterator )
                return m_selectionIterator->rawData();
            else
                return 0;
        }


protected:
    KisPaintDevice *m_device;

    inline void advance(int n){if (m_selectionIterator) for(int i=0; i< n; i++) ++(*m_selectionIterator);};

    void setSelectionIterator(_iTp *si){m_selectionIterator = si;};

    _iTp *m_underlyingIterator;
    _iTp *m_selectionIterator;
};

#endif
