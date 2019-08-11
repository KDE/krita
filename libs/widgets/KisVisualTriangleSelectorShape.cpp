/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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
#include "KisVisualTriangleSelectorShape.h"

#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QVector>
#include <QVBoxLayout>
#include <QList>
#include <QPolygon>
#include <QtMath>

#include <KSharedConfig>
#include <KConfigGroup>

#include "KoColorConversions.h"
#include "KoColorDisplayRendererInterface.h"
#include "KoChannelInfo.h"
#include <KoColorModelStandardIds.h>
#include <QPointer>
#include "kis_signal_compressor.h"
#include "kis_debug.h"
#include "kis_global.h"

KisVisualTriangleSelectorShape::KisVisualTriangleSelectorShape(QWidget *parent,
                                                                 Dimensions dimension,
                                                                 ColorModel model,
                                                                 const KoColorSpace *cs,
                                                                 int channel1, int channel2,
                                                                 const KoColorDisplayRendererInterface *displayRenderer,
                                                                 int barwidth)
    : KisVisualColorSelectorShape(parent, dimension, model, cs, channel1, channel2, displayRenderer)
{
    //qDebug() << "creating KisVisualTriangleSelectorShape" << this;
    m_barWidth = barwidth;
    setTriangle();
}

KisVisualTriangleSelectorShape::~KisVisualTriangleSelectorShape()
{
    //qDebug() << "deleting KisVisualTriangleSelectorShape" << this;
}

void KisVisualTriangleSelectorShape::setBorderWidth(int width)
{
    m_barWidth = width;
}

QRect KisVisualTriangleSelectorShape::getSpaceForSquare(QRect geom)
{
    return geom;
}

QRect KisVisualTriangleSelectorShape::getSpaceForCircle(QRect geom)
{
    return geom;
}

QRect KisVisualTriangleSelectorShape::getSpaceForTriangle(QRect geom)
{
    return geom;
}
void KisVisualTriangleSelectorShape::setTriangle()
{
    QPoint apex = QPoint (width()*0.5,0);
    QPolygon triangle;
    triangle<< QPoint(0,height()) << apex << QPoint(width(),height()) << QPoint(0,height());
    m_triangle = triangle;
    QLineF a(triangle.at(0),triangle.at(1));
    QLineF b(triangle.at(0),triangle.at(2));
    QLineF ap(triangle.at(2), a.pointAt(0.5));
    QLineF bp(triangle.at(1), b.pointAt(0.5));
    QPointF intersect;
    ap.intersect(bp,&intersect);
    m_center = intersect;
    QLineF r(triangle.at(0), intersect);
    m_radius = r.length();
}

QPointF KisVisualTriangleSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate)
{
    qreal offset=7.0;//the offset is so we get a nice little border that allows selecting extreme colors better.
    qreal yOffset = (cos(kisDegreesToRadians(30))*offset)*2;
    qreal xOffset = qFloor(sin(kisDegreesToRadians(30))*offset);
    qreal y = qMax(qMin((coordinate.y()*(height()-yOffset-offset))+yOffset, (qreal)height()-offset),yOffset);

    qreal triWidth = width();
    qreal horizontalLineLength = ((y-yOffset)*(2./sqrt(3.)));
    qreal horizontalLineStart = (triWidth*0.5)-(horizontalLineLength*0.5);
    qreal relativeX = qMin(coordinate.x()*(horizontalLineLength), horizontalLineLength);
    qreal x = qMax(relativeX + horizontalLineStart + xOffset, horizontalLineStart+xOffset);
    if (y<=yOffset){
        x = 0.5*width();
    }

    return QPointF(x,y);
}

QPointF KisVisualTriangleSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPoint coordinate)
{
    //default implementation: gotten from the kotrianglecolorselector/kis_color_selector_triangle.
    qreal x = 0.5;
    qreal y = 0.5;
    qreal offset=7.0; //the offset is so we get a nice little border that allows selecting extreme colors better.
    qreal yOffset = (cos(kisDegreesToRadians(30))*offset)*2;
    qreal xOffset = qFloor(sin(kisDegreesToRadians(30))*offset);

    y = qMin(qMax((qreal)coordinate.y()-yOffset, 0.0)/(height()-yOffset-offset), 1.0);

    qreal triWidth = width();
    qreal horizontalLineLength = ((qreal)coordinate.y()-yOffset)*(2./sqrt(3.));
    qreal horizontalLineStart = (triWidth*0.5)-(horizontalLineLength*0.5);

    qreal relativeX = qMax((qreal)coordinate.x()-xOffset-horizontalLineStart,0.0);
    x = qMin(relativeX/horizontalLineLength, 1.0);
    if (coordinate.y()<=yOffset){
        x = 0.5;
    }
    return QPointF(x, y);
}

QRegion KisVisualTriangleSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(m_triangle);
    //QRegion mask =  QRegion();
    //if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
    //    mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2)));
    //}
    return mask;
}

void KisVisualTriangleSelectorShape::resizeEvent(QResizeEvent *)
{
    //qDebug() << this << "KisVisualTriangleSelectorShape::resizeEvent(QResizeEvent *)";
    setTriangle();
    forceImageUpdate();
}

void KisVisualTriangleSelectorShape::drawCursor()
{
    //qDebug() << this << "KisVisualTriangleSelectorShape::drawCursor: image needs update" << imagesNeedUpdate();
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QImage fullSelector = getImageMap();
    QColor col = getColorFromConverter(getCurrentColor());
    QPainter painter;
    painter.begin(&fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen pen;
    pen.setWidth(10);
    painter.setPen(pen);
    painter.drawPolygon(m_triangle);
    painter.restore();

    //QPainterPath path;
    QBrush fill;
    fill.setStyle(Qt::SolidPattern);

    int cursorwidth = 5;
    //QRect innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
    /*if(m_type==KisVisualTriangleSelectorShape::borderMirrored){
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        QPoint mirror(innerRect.center().x()+(innerRect.center().x()-cursorPoint.x()),cursorPoint.y());
        painter.drawEllipse(mirror, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1, cursorwidth-1);
        painter.drawEllipse(mirror, cursorwidth-1, cursorwidth-1);

    } else {*/
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1.0, cursorwidth-1.0);
    //}
    painter.end();
    setFullImage(fullSelector);
}
