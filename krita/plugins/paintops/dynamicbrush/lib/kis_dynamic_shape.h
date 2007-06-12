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

class KisDynamicColoring;
class KisPaintInformation;
class KisPainter;

class DYNAMIC_BRUSH_EXPORT KisDynamicShape {
    public:
        KisDynamicShape() {}
        virtual ~KisDynamicShape() {}
    public:
        virtual QRect rect() =0;
        virtual KisDynamicShape* clone() const = 0;
        virtual void rotate(double r) = 0;
        virtual void startPainting(KisPainter* m_painter);
        virtual void endPainting();
        /**
         * Call this function to resize the shape.
         * @param xs horizontal scaling
         * @param ys vertical scaling
         */
        virtual void resize(double xs, double ys) = 0;
        virtual void paintAt(const QPointF &pos, const KisPaintInformation& info, KisDynamicColoring* coloringsrc) = 0;
    protected:
        KisPainter* painter() { return m_painter;}
    private:
        KisPainter* m_painter;

};

#endif
