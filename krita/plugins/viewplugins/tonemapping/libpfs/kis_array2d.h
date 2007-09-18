/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KIS_ARRAY_2D_H_
#define _KIS_ARRAY_2D_H_

#include "array2d.h"

#include "kis_types.h"

namespace pfs
{
    /**
     * This class is a replacement for the Array2DImpl of libpfs which use a KisPaintDevice.
     */
    class Array2DImpl : public Array2D
    {
        public:
            Array2DImpl( int cols, int rows);
            Array2DImpl( int cols, int rows, int index, KisPaintDeviceSP device );
            ~Array2DImpl();
            int getCols() const;
            int getRows() const;
            float& operator()( int col, int row );
            const float& operator()( int col, int row ) const;
            float& operator()( int index );
            const float& operator()( int index ) const;
        private:
            void init( int cols, int rows, int index, KisPaintDeviceSP device );
        private:
            struct Private;
            Private* const d;
    };
    
    
}

#endif
