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

#include "krs_painter.h"

#include <kis_paint_layer.h>
#include <kis_paintop_registry.h>
#include <kis_painter.h>

#include "krs_brush.h"
#include "krs_color.h"

namespace Kross {

namespace KritaCore {

Painter::Painter(KisPaintLayerSP layer)
    : Kross::Api::Class<Painter>("KritaPainter"), m_layer(layer),m_painter(new KisPainter(layer->paintDevice()))
{
    addFunction("paintPolyline", &Painter::paintPolyline, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant::List") << Kross::Api::Argument("Kross::Api::Variant::List") );
    addFunction("paintLine", &Painter::paintLine, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintBezierCurve", &Painter::paintBezierCurve, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintEllipse", &Painter::paintEllipse, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintPolygon", &Painter::paintPolygon, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::List") << Kross::Api::Argument("Kross::Api::Variant::List") );
    addFunction("paintRect", &Painter::paintRect, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintAt", &Painter::paintAt, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("setPaintColor", &Painter::setPaintColor, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Color") );
    addFunction("setBrush", &Painter::setBrush, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Brush") );
    addFunction("setPaintOp", &Painter::setPaintOp, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
}


Painter::~Painter()
{
    delete m_painter;
}

Kross::Api::Object::Ptr Painter::paintPolyline(Kross::Api::List::Ptr args)
{
    QValueList<QVariant> pointsX = Kross::Api::Variant::toList( args->item(0) );
    QValueList<QVariant> pointsY = Kross::Api::Variant::toList( args->item(1) );
    if(pointsX.size() != pointsY.size())
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("the two lists should have the same size.") );
    }
    m_painter->paintPolyline( createPointsVector( pointsX, pointsY));
    return 0;
}

Kross::Api::Object::Ptr Painter::paintLine(Kross::Api::List::Ptr args)
{
    double x1 = Kross::Api::Variant::toVariant(args->item(0)).toDouble();
    double y1 = Kross::Api::Variant::toVariant(args->item(1)).toDouble();
    double p1 = Kross::Api::Variant::toVariant(args->item(2)).toDouble();
    double x2 = Kross::Api::Variant::toVariant(args->item(3)).toDouble();
    double y2 = Kross::Api::Variant::toVariant(args->item(4)).toDouble();
    double p2 = Kross::Api::Variant::toVariant(args->item(5)).toDouble();
    m_painter->paintLine(KisPoint( x1, y1), p1, 0.0, 0.0, KisPoint( x2, y2 ), p2, 0.0, 0.0 );
    return 0;
}

Kross::Api::Object::Ptr Painter::paintBezierCurve(Kross::Api::List::Ptr args)
{
    double x1 = Kross::Api::Variant::toVariant(args->item(0)).toDouble();
    double y1 = Kross::Api::Variant::toVariant(args->item(1)).toDouble();
    double p1 = Kross::Api::Variant::toVariant(args->item(2)).toDouble();
    double cx1 = Kross::Api::Variant::toVariant(args->item(3)).toDouble();
    double cy1 = Kross::Api::Variant::toVariant(args->item(4)).toDouble();
    double cx2 = Kross::Api::Variant::toVariant(args->item(5)).toDouble();
    double cy2 = Kross::Api::Variant::toVariant(args->item(6)).toDouble();
    double x2 = Kross::Api::Variant::toVariant(args->item(7)).toDouble();
    double y2 = Kross::Api::Variant::toVariant(args->item(8)).toDouble();
    double p2 = Kross::Api::Variant::toVariant(args->item(9)).toDouble();
    m_painter->paintBezierCurve( KisPoint(x1,y1), p1, 0.0, 0.0, KisPoint(cx1,cy1), KisPoint(cx2,cy2), KisPoint(x2,y2), p2, 0.0, 0.0);
    return 0;
}

Kross::Api::Object::Ptr Painter::paintEllipse(Kross::Api::List::Ptr args)
{
    double x1 = Kross::Api::Variant::toVariant(args->item(0)).toDouble();
    double y1 = Kross::Api::Variant::toVariant(args->item(1)).toDouble();
    double x2 = Kross::Api::Variant::toVariant(args->item(2)).toDouble();
    double y2 = Kross::Api::Variant::toVariant(args->item(3)).toDouble();
    double p1 = Kross::Api::Variant::toVariant(args->item(4)).toDouble();
    m_painter->paintEllipse( KisPoint(x1,y1), KisPoint(x2,y2), p1, 0.0, 0.0 );
    return 0;
}

Kross::Api::Object::Ptr Painter::paintPolygon(Kross::Api::List::Ptr args)
{
    QValueList<QVariant> pointsX = Kross::Api::Variant::toList( args->item(0) );
    QValueList<QVariant> pointsY = Kross::Api::Variant::toList( args->item(1) );
    if(pointsX.size() != pointsY.size())
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("the two lists should have the same size.") );
    }
    m_painter->paintPolygon( createPointsVector(pointsX, pointsY));
    return 0;
}

Kross::Api::Object::Ptr Painter::paintRect(Kross::Api::List::Ptr args)
{
    double x1 = Kross::Api::Variant::toVariant(args->item(0)).toDouble();
    double y1 = Kross::Api::Variant::toVariant(args->item(1)).toDouble();
    double x2 = Kross::Api::Variant::toVariant(args->item(2)).toDouble();
    double y2 = Kross::Api::Variant::toVariant(args->item(3)).toDouble();
    double p1 = Kross::Api::Variant::toVariant(args->item(4)).toDouble();
    m_painter->paintRect( KisPoint(x1, y1), KisPoint(x2,y2), p1, 0, 0);
    return 0;
}

Kross::Api::Object::Ptr Painter::paintAt(Kross::Api::List::Ptr args)
{
    double x1 = Kross::Api::Variant::toVariant(args->item(0)).toDouble();
    double y1 = Kross::Api::Variant::toVariant(args->item(1)).toDouble();
    double p1 = Kross::Api::Variant::toVariant(args->item(2)).toDouble();
    m_painter->paintAt( KisPoint( x1, y1 ), p1, 0.0, 0.0);
    return 0;
}

Kross::Api::Object::Ptr Painter::setPaintColor(Kross::Api::List::Ptr args)
{
//     kdDebug() << "setPaintColor : " << args->item(0)->getClassName() << endl;
    Color* c = (Color*)args->item(0).data();
    m_painter->setPaintColor( KisColor(c->toQColor(), paintLayer()->paintDevice()->colorSpace() ));
    return 0;
}

Kross::Api::Object::Ptr Painter::setBrush(Kross::Api::List::Ptr args)
{
    Brush* b = (Brush*)args->item(0).data();
    m_painter->setBrush( b->getBrush());
    return 0;
}
Kross::Api::Object::Ptr Painter::setPaintOp(Kross::Api::List::Ptr args)
{
    QString id = Kross::Api::Variant::toString(args->item(0));
    KisPaintOp* op = KisPaintOpRegistry::instance()->paintOp( id, m_painter );
    m_painter->setPaintOp( op );
    return 0;
}

}

}
