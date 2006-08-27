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
#include <kis_paint_layer.h>
#include <kis_paintop_registry.h>

#include "krs_brush.h"
#include "krs_color.h"
#include "krs_pattern.h"

using namespace Kross::KritaCore;

Painter::Painter(KisPaintLayerSP layer)
    : QObject()
    , m_layer(layer)
    , m_painter(new KisPainter(layer->paintDevice()))
    , m_threshold(1)
{
    setObjectName("KritaPainter");
}


Painter::~Painter()
{
    delete m_painter;
}

#if 0
void Painter::convolve()
{
    KisConvolutionPainter* cp = new KisConvolutionPainter(m_painter->device());
    QRect rect;
    KisKernel kernel;
    kernel.factor = Kross::Api::Variant::toInt(args->item(1));
    kernel.offset = Kross::Api::Variant::toInt(args->item(2));

    uint borderop = 3;
    if( args.count() > 3 )
        borderop = Kross::Api::Variant::toUInt(args->item(3));
    uint channelsFlag = KoChannelInfo::FLAG_COLOR;
    if( args.count() > 4 )
        channelsFlag = Kross::Api::Variant::toUInt(args->item(4));
    if( args.count() > 5) {
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

    QList<QVariant> kernelH = Kross::Api::Variant::toList( args->item(0) );

    QVariant firstlineVariant = *kernelH.begin();
    if(firstlineVariant.type() != QVariant::List)
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(i18n("An error has occured in %1",QString("applyConvolution"))) );

    QList<QVariant> firstline = firstlineVariant.toList();

    kernel.height = kernelH.size();
    kernel.width = firstline.size();
    kernel.data = new qint32[kernel.height * kernel.width];

    uint i = 0;
    for(QList<QVariant>::iterator itK = kernelH.begin(); itK != kernelH.end(); itK++, i ++ ) {
        QVariant lineVariant = *kernelH.begin();
        if(lineVariant.type() != QVariant::List)
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(i18n("An error has occured in %1",QString("applyConvolution"))) );
        QList<QVariant> line = firstlineVariant.toList();
        if(line.size() != kernel.width)
            throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(i18n("An error has occured in %1",QString("applyConvolution"))) );
        uint j = 0;
        for(QList<QVariant>::iterator itLine = line.begin(); itLine != line.end(); itLine++, j ++ )
            kernel.data[ j + i * kernel.width ] = (*itLine).toInt();
    }
    cp->applyMatrix(KisKernelSP(&kernel), rect.x(), rect.y(), rect.width(), rect.height(), (KisConvolutionBorderOp)borderop, (KoChannelInfo::enumChannelFlags) channelsFlag);

    delete[] kernel.data;
    return Kross::Api::Object::Ptr(0);
}
#endif

void Painter::setFillThreshold(int threshold)
{
    m_threshold = threshold;
}

void Painter::fillColor(uint x, uint y)
{
    KisFillPainter* fp = createFillPainter();
    fp->fillColor(x, y);
}

void Painter::fillPattern(uint x, uint y)
{
    KisFillPainter* fp = createFillPainter();
    fp->fillPattern(x, y);
}

void Painter::setFillStyle(uint style)
{
    KisPainter::FillStyle fillstyle;
    switch(style) {
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
}

void Painter::setOpacity(uint opacity)
{
    m_painter->setOpacity( (quint8)opacity );
}

void Painter::setStrokeStyle(uint style)
{
    KisPainter::StrokeStyle strokestyle;
    switch(style) {
        case 1:
            strokestyle = KisPainter::StrokeStyleBrush;
            break;
        default:
            strokestyle = KisPainter::StrokeStyleNone;
    }
    m_painter->setStrokeStyle(strokestyle);
}

void Painter::setDuplicateOffset(double x1, double y1)
{
    m_painter->setDuplicateOffset(KisPoint(x1,y1));
}

void Painter::paintPolyline(QVariantList pointsX, QVariantList pointsY)
{
    if(pointsX.size() != pointsY.size()) {
        kWarning() << QString("The two lists of points should have the same size.") << endl;
        return;
    }
    m_painter->paintPolyline( createPointsVector( pointsX, pointsY));
}

void Painter::paintLine(double x1, double y1, double p1, double x2, double y2, double p2)
{
    m_painter->paintLine(KisPoint( x1, y1), p1, 0.0, 0.0, KisPoint( x2, y2 ), p2, 0.0, 0.0 );
}

void Painter::paintBezierCurve(double x1, double y1, double p1, double cx1, double cy1, double cx2, double cy2, double x2, double y2, double p2)
{
    m_painter->paintBezierCurve( KisPoint(x1,y1), p1, 0.0, 0.0, KisPoint(cx1,cy1), KisPoint(cx2,cy2), KisPoint(x2,y2), p2, 0.0, 0.0);
}

void Painter::paintEllipse(double x1, double y1, double x2, double y2, double pressure)
{
    m_painter->paintEllipse( KisPoint(x1,y1), KisPoint(x2,y2), pressure, 0.0, 0.0 );
}

void Painter::paintPolygon(QVariantList pointsX, QVariantList pointsY)
{
    if(pointsX.size() != pointsY.size()) {
        kWarning() << "The two lists of points should have the same size." << endl;
        return;
    }
    m_painter->paintPolygon( createPointsVector(pointsX, pointsY) );
}

void Painter::paintRect(double x, double y, double width, double height, double pressure)
{
    m_painter->paintRect( KisPoint(x, y), KisPoint(width, height), pressure, 0, 0);
}

void Painter::paintAt(double x, double y, double pressure)
{
    m_painter->paintAt( KisPoint( x, y ), pressure, 0.0, 0.0);
}

void Painter::setPaintColor(QObject* color)
{
    Color* c = dynamic_cast< Color* >(color);
    if(c) m_painter->setPaintColor( KoColor(c->toQColor(), paintLayer()->paintDevice()->colorSpace() ));
}

void Painter::setBackgroundColor(QObject* color)
{
    Color* c = dynamic_cast< Color* >(color);
    if(c) m_painter->setBackgroundColor( KoColor(c->toQColor(), paintLayer()->paintDevice()->colorSpace() ));
}

void Painter::setPattern(QObject* pattern)
{
    Pattern* p = dynamic_cast< Pattern* >(pattern);
    if(p) m_painter->setPattern( p->getPattern() );
}

void Painter::setBrush(QObject* brush)
{
    Brush* b = dynamic_cast< Brush* >(brush);
    if(b) m_painter->setBrush( b->getBrush() );
}

void Painter::setPaintOp(const QString& paintopname)
{
    KisPaintOp* op = KisPaintOpRegistry::instance()->paintOp( paintopname, 0, m_painter );
    if(op) m_painter->setPaintOp( op );
}

#include "krs_painter.moc"
