/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef _KIS_DYNAMIC_SHAPE_H_
#define _KIS_DYNAMIC_SHAPE_H_

#include "dynamicbrush_export.h"

#include <kis_types.h>
#include <QRect>

class KisAutobrushShape;
class KisDynamicColoring;
class KisPaintInformation;

class DYNAMIC_BRUSH_EXPORT KisDynamicShape {
    public:
        KisDynamicShape() {}
        virtual ~KisDynamicShape() {}
    public:
        virtual QRect rect() =0;
        virtual KisDynamicShape* clone() const = 0;
        virtual void rotate(double r) = 0;
        /**
         * Call this function to resize the shape.
         * @param xs horizontal scaling
         * @param ys vertical scaling
         */
        virtual void resize(double xs, double ys) = 0;
        /**
         * Call this function to create the stamp to apply on the paint device
         * @param stamp the temporary paint device on which the shape will draw the stamp
         * @param coloringsrc the color source to use for the stamp
         */
        virtual void createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc, const QPointF &pos, const KisPaintInformation& info) =0;
};

#endif
