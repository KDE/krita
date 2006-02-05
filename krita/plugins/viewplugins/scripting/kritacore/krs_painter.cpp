/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#include <kis_convolution_painter.h>
#include <kis_fill_painter.h>
#include <kis_paint_layer.h>
#include <kis_paintop_registry.h>
#include <kis_painter.h>

#include "krs_brush.h"
#include "krs_color.h"
#include "krs_pattern.h"

namespace Kross {

namespace KritaCore {

Painter::Painter(KisPaintLayerSP layer)
    : Kross::Api::Class<Painter>("KritaPainter"), m_layer(layer),m_painter(new KisPainter(layer->paintDevice())),m_threshold(1)
{
    // convolution
    addFunction("convolve", &Painter::convolve);
    // Fill specific
    addFunction("setFillThreshold", &Painter::setFillThreshold);
    addFunction("fillColor", &Painter::fillColor);
    addFunction("fillPattern", &Painter::fillPattern);

    // Painting operations
    addFunction("paintPolyline", &Painter::paintPolyline, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant::List") << Kross::Api::Argument("Kross::Api::Variant::List") );
    addFunction("paintLine", &Painter::paintLine, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintBezierCurve", &Painter::paintBezierCurve, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintEllipse", &Painter::paintEllipse, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintPolygon", &Painter::paintPolygon, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::List") << Kross::Api::Argument("Kross::Api::Variant::List") );
    addFunction("paintRect", &Painter::paintRect, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("paintAt", &Painter::paintAt, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("setBackgroundColor", &Painter::setBackgroundColor, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Color") );
    addFunction("setPaintColor", &Painter::setPaintColor, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Color") );
    
    // Color operations
    addFunction("setPattern", &Painter::setPattern, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Pattern") );
    addFunction("setBrush", &Painter::setBrush, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Brush") );
    
    // How is painting done operations
    addFunction("setPaintOp", &Painter::setPaintOp, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
    // Special settings
    addFunction("setDuplicateOffset", &Painter::setDuplicateOffset, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    
    // Style operation
    addFunction("setOpacity", &Painter::setOpacity, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("setStrokeStyle", &Painter::setStrokeStyle, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("setFillStyle", &Painter::setFillStyle, Kross::Api::ArgumentList() <<  Kross::Api::Argument("Kross::Api::Variant") );
}


Painter::~Painter()
{
    delete m_painter;
}

Kross::Api::Object::Ptr Painter::convolve(Kross::Api::List::Ptr args)
{
    KisConvolutionPainter* cp = new KisConvolutionPainter(m_painter->device());
    QRect rect;
    KisKernel kernel;
    kernel.factor = Kross::Api::Variant::toInt(args->item(1));
    kernel.offset = Kross::Api::Variant::toInt(args->item(2));
    
    uint borderop = 3;
    if( args.count() > 3 )
    {
        borderop = Kross::Api::Variant::toUInt(args->item(3));
    }
    uint channelsFlag = KisChannelInfo::FLAG_COLOR;
    if( args.count() > 4 )
    {
        channelsFlag = Kross::Api::Variant::toUInt(args->item(4));
    }
    if( args.count() > 5)
    {
        uint x = Kross::Api::Variant::toUInt(args->item(5));
        uint y = Kross::Api::Variant::toUInt(args->item(6));
        uint w = Kross::Api::Variant::toUInt(args->item(7));
        uint h = Kross::Api::Variant::toUInt(args->item(8));
        rect = QRect(x,y,w,h);
    } else {
        QRect r1 = paintLayer()->paintDevice()->extent();
        QRect r2 = paintLayer()->image()->bounds();
        rect = r1.intersect(r2);
    }
    
    QValueList<QVariant> kernelH = Kross::Api::Variant::toList( args->item(0) );
    
    QVariant firstlineVariant = *kernelH.begin();
    if(firstlineVariant.type() != QVariant::List)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(i18n("An error has occured in %1").arg("applyConvolution")) );
    }
    
    QValueList<QVariant> firstline = firstlineVariant.toList();
    
    kernel.height = kernelH.size();
    kernel.width = firstline.size();
    
    kernel.data = new Q_INT32[kernel.height * kernel.width];
    
    uint i = 0;
    for(QValueList<QVariant>::iterator itK = kernelH.begin(); itK != kernelH.end(); itK++, i ++ )
    {
        QVariant lineVariant = *kernelH.begin();
        if(lineVariant.type() != QVariant::List)
        {
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(i18n("An error has occured in %1").arg("applyConvolution")) );
        }
        QValueList<QVariant> line = firstlineVariant.toList();
        if(line.size() != kernel.width)
        {
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(i18n("An error has occured in %1").arg("applyConvolution")) );
        }
        uint j = 0;
        for(QValueList<QVariant>::iterator itLine = line.begin(); itLine != line.end(); itLine++, j ++ )
        {
            kernel.data[ j + i * kernel.width ] = (*itLine).toInt();
        }
    }
    cp->applyMatrix(&kernel, rect.x(), rect.y(), rect.width(), rect.height(), (KisConvolutionBorderOp)borderop, (KisChannelInfo::enumChannelFlags) channelsFlag);
    
    delete[] kernel.data;
    return 0;
}


KisFillPainter* Painter::createFillPainter()
{
    KisFillPainter* fp = new KisFillPainter(m_painter->device());
    fp->setBrush( m_painter->brush() );
    fp->setFillColor( m_painter->fillColor() );
    fp->setPaintColor( m_painter->paintColor() );
    fp->setFillStyle( m_painter->fillStyle() );
    fp->setOpacity( m_painter->opacity() );
    fp->setPattern( m_painter->pattern() );
    return fp;
}

Kross::Api::Object::Ptr Painter::setFillThreshold(Kross::Api::List::Ptr args)
{
    m_threshold = Kross::Api::Variant::toInt(args->item(0));
    return 0;
}
Kross::Api::Object::Ptr Painter::fillColor(Kross::Api::List::Ptr args)
{
    KisFillPainter* fp = createFillPainter();
    uint x = Kross::Api::Variant::toUInt(args->item(0));
    uint y = Kross::Api::Variant::toUInt(args->item(1));

    fp->fillColor( x, y );
    return 0;
}
Kross::Api::Object::Ptr Painter::fillPattern(Kross::Api::List::Ptr args)
{
    KisFillPainter* fp = createFillPainter();
    uint x = Kross::Api::Variant::toUInt(args->item(0));
    uint y = Kross::Api::Variant::toUInt(args->item(1));
    fp->fillPattern(x, y);
    return 0;
}

Kross::Api::Object::Ptr Painter::setStrokeStyle(Kross::Api::List::Ptr args)
{
    uint style = Kross::Api::Variant::toVariant(args->item(0)).toUInt();
    KisPainter::StrokeStyle strokestyle;
    switch(style)
    {
        case 1:
            strokestyle = KisPainter::StrokeStyleBrush;
            break;
        default:
            strokestyle = KisPainter::StrokeStyleNone;
    }
    m_painter->setStrokeStyle(strokestyle);
    return 0;
}

Kross::Api::Object::Ptr Painter::setFillStyle(Kross::Api::List::Ptr args)
{
    uint style = Kross::Api::Variant::toVariant(args->item(0)).toUInt();
    KisPainter::FillStyle fillstyle;
    switch(style)
    {
        case 1:
            fillstyle = KisPainter::FillStyleForegroundColor;
            break;
        case 2:
            fillstyle = KisPainter::FillStyleBackgroundColor;
            break;
        case 3:
            fillstyle = KisPainter::FillStylePattern;
            break;
        default:
            fillstyle = KisPainter::FillStyleNone;
    }
    m_painter->setFillStyle(fillstyle);
    return 0;
}

Kross::Api::Object::Ptr Painter::setOpacity(Kross::Api::List::Ptr args)
{
    Q_UINT8 opacity = Kross::Api::Variant::toVariant(args->item(0)).toUInt();
    m_painter->setOpacity(opacity);
    return 0;
}

Kross::Api::Object::Ptr Painter::setDuplicateOffset(Kross::Api::List::Ptr args)
{
    double x1 = Kross::Api::Variant::toVariant(args->item(0)).toDouble();
    double y1 = Kross::Api::Variant::toVariant(args->item(1)).toDouble();
    m_painter->setDuplicateOffset(KisPoint(x1,y1));
    return 0;
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

Kross::Api::Object::Ptr Painter::setBackgroundColor(Kross::Api::List::Ptr args)
{
    Color* c = (Color*)args->item(0).data();
    m_painter->setBackgroundColor( KisColor(c->toQColor(), paintLayer()->paintDevice()->colorSpace() ));
    return 0;
}

Kross::Api::Object::Ptr Painter::setPaintColor(Kross::Api::List::Ptr args)
{
    Color* c = (Color*)args->item(0).data();
    m_painter->setPaintColor( KisColor(c->toQColor(), paintLayer()->paintDevice()->colorSpace() ));
    return 0;
}

Kross::Api::Object::Ptr Painter::setPattern(Kross::Api::List::Ptr args)
{
    Pattern* p = (Pattern*)args->item(0).data();
    m_painter->setPattern( p->getPattern());
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
    KisPaintOp* op = KisPaintOpRegistry::instance()->paintOp( id, 0, m_painter );
    m_painter->setPaintOp( op );
    return 0;
}

}

}
