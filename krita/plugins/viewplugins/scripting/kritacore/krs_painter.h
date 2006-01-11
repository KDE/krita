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

#ifndef KROSS_KRITACOREKRS_PAINTER_H
#define KROSS_KRITACOREKRS_PAINTER_H

#include <api/class.h>

#include <kis_point.h>
#include <kis_types.h>

class KisPainter;

namespace Kross {

namespace KritaCore {

class Painter : public Kross::Api::Class<Painter>
{
    public:
        explicit Painter(KisPaintLayerSP layer);
        ~Painter();
    private:
        // Painting operations
        Kross::Api::Object::Ptr paintPolyline(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr paintLine(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr paintBezierCurve(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr paintEllipse(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr paintPolygon(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr paintRect(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr paintAt(Kross::Api::List::Ptr args);
        // Color operations
        Kross::Api::Object::Ptr setPaintColor(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr setBackgroundColor(Kross::Api::List::Ptr args);
        // How is painting done operations
        Kross::Api::Object::Ptr setPattern(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr setBrush(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr setPaintOp(Kross::Api::List::Ptr args);
        // Spetial settings
        Kross::Api::Object::Ptr setDuplicateOffset(Kross::Api::List::Ptr args);
        // Style operation
        Kross::Api::Object::Ptr setOpacity(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr setStrokeStyle(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr setFillStyle(Kross::Api::List::Ptr args);
    protected:
        inline KisPaintLayerSP paintLayer() { return m_layer; }
    private:
        inline vKisPoint createPointsVector( QValueList<QVariant> xs, QValueList<QVariant> ys)
        {
            vKisPoint a;
            QValueList<QVariant>::iterator itx = xs.begin();
            QValueList<QVariant>::iterator ity = ys.begin();
            for(; itx != xs.end(); ++itx, ++ity)
            {
                a.push_back(KisPoint( (*itx).toDouble(), (*ity).toDouble()));
            }
            return a;
        }
    private:
        KisPaintLayerSP m_layer;
        KisPainter* m_painter;
};

}

}

#endif
