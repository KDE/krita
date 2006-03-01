/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_KRITACOREKRS_BRUSH_H
#define KROSS_KRITACOREKRS_BRUSH_H

#include <api/class.h>

class KisBrush;

namespace Kross {

namespace KritaCore {

class Brush : public Kross::Api::Class<Brush>{
    public:
        /**
         * @param sharedBrush tell if the brush should be deleted or not when this object is deleted
         */
        Brush(KisBrush*, bool sharedBrush );
        ~Brush();
    public:
        inline KisBrush* getBrush() { return m_brush; }
    private:
        KisBrush* m_brush;
        bool m_sharedBrush;
};

}

}

#endif
