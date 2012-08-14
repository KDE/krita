/*
   Calligra Report Engine
   Copyright (C) 2011, 2012 by Dag Andersen (danders@get2net.dk)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoOdtFrameReportLine.h"
#include <KoXmlWriter.h>
#include <KoDpi.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include "renderobjects.h"

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QImage>

#include <kmimetype.h>
#include <kdebug.h>

KoOdtFrameReportLine::KoOdtFrameReportLine(OROLine *primitive)
    : KoOdtFrameReportPrimitive(primitive)
{
}

KoOdtFrameReportLine::~KoOdtFrameReportLine()
{
}

OROLine *KoOdtFrameReportLine::line() const
{
    return static_cast<OROLine*>(m_primitive);
}

void KoOdtFrameReportLine::createStyle(KoGenStyles &coll)
{
    KoGenStyle ps(KoGenStyle::ParagraphStyle, "paragraph");
    m_paragraphStyleName = coll.insert(ps, "P");

    KoGenStyle gs(KoGenStyle::GraphicStyle, "graphic");
    gs.addProperty("draw:fill", "none");
    gs.addPropertyPt("fo:margin", 0);
    gs.addProperty("style:horizontal-pos", "from-left");
    gs.addProperty("style:horizontal-rel", "page");
    gs.addProperty("style:vertical-pos", "from-top");
    gs.addProperty("style:vertical-rel", "page");
    gs.addProperty("style:wrap", "dynamic");
    gs.addPropertyPt("style:wrap-dynamic-threshold", 0);

    QPen pen;
    qreal weight = line()->lineStyle().weight;
    if (weight < 1.0) {
        weight = 1.0;
    }
    pen.setWidthF(weight);
    pen.setColor(line()->lineStyle().lineColor);
    pen.setStyle(line()->lineStyle().style);
    KoOdfGraphicStyles::saveOdfStrokeStyle(gs, coll, pen);

    m_frameStyleName = coll.insert(gs, "F");
    kDebug()<<coll;
}

void KoOdtFrameReportLine::createBody(KoXmlWriter *bodyWriter) const
{
    // convert to inches
    qreal sx = INCH_TO_POINT(line()->startPoint().x() / KoDpi::dpiX());
    qreal sy = INCH_TO_POINT(line()->startPoint().y() / KoDpi::dpiY());
    qreal ex = INCH_TO_POINT(line()->endPoint().x() / KoDpi::dpiX());
    qreal ey = INCH_TO_POINT(line()->endPoint().y() / KoDpi::dpiY());
    qreal width = ex - sx;
    qreal height = ey - sy;

    kDebug()<<line()->startPoint()<<line()->endPoint();

    bodyWriter->startElement("draw:rect");
    bodyWriter->addAttribute("draw:id", itemName());
    bodyWriter->addAttribute("xml:id", itemName());
    bodyWriter->addAttribute("draw:name", itemName());
    bodyWriter->addAttribute("text:anchor-type", "page");
    bodyWriter->addAttribute("text:anchor-page-number", pageNumber());
    bodyWriter->addAttribute("draw:style-name", m_frameStyleName);
    bodyWriter->addAttribute("draw:z-index", "3");

    if (height == 0.0 && width >= 0.0) {
        // just a horizontal line
        bodyWriter->addAttributePt("svg:x", sx);
        bodyWriter->addAttributePt("svg:y", sy);
        bodyWriter->addAttributePt("svg:width", width);
        bodyWriter->addAttributePt("svg:height", 0.0);
    } else {
        // rotate
        qreal l = sqrt(width*width + height*height);
        bodyWriter->addAttributePt("svg:width", l);
        bodyWriter->addAttributePt("svg:height", 0.0);
        qreal sina = height / l;
        qreal cosa = width / l;
        QTransform rotate(cosa, sina, -sina, cosa, sx, sy);
        bodyWriter->addAttribute("draw:transform", KoOdfGraphicStyles::saveTransformation(rotate));
    }

    bodyWriter->endElement(); // draw:frame
}
