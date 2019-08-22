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
#include "KisVisualEllipticalSelectorShape.h"

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

KisVisualEllipticalSelectorShape::KisVisualEllipticalSelectorShape(QWidget *parent,
                                                                 Dimensions dimension,
                                                                 ColorModel model,
                                                                 const KoColorSpace *cs,
                                                                 int channel1, int channel2,
                                                                 const KoColorDisplayRendererInterface *displayRenderer,
                                                                 int barWidth,
                                                                 singelDTypes d)
    : KisVisualColorSelectorShape(parent, dimension, model, cs, channel1, channel2, displayRenderer)
{
    //qDebug() << "creating KisVisualEllipticalSelectorShape" << this;
    m_type = d;
    m_barWidth = barWidth;
}

KisVisualEllipticalSelectorShape::~KisVisualEllipticalSelectorShape()
{
    //qDebug() << "deleting KisVisualEllipticalSelectorShape" << this;
}

QSize KisVisualEllipticalSelectorShape::sizeHint() const
{
    return QSize(180,180);
}
void KisVisualEllipticalSelectorShape::setBorderWidth(int width)
{
    m_barWidth = width;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForSquare(QRect geom)
{
    int sizeValue = qMin(width(),height());
    QRect b(geom.left(), geom.top(), sizeValue, sizeValue);
    QLineF radius(b.center(), QPointF(b.left()+m_barWidth, b.center().y()) );
    radius.setAngle(135);
    QPointF tl = radius.p2();
    radius.setAngle(315);
    QPointF br = radius.p2();
    QRect r(tl.toPoint(), br.toPoint());
    return r;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForCircle(QRect geom)
{
    int sizeValue = qMin(width(),height());
    QRect b(geom.left(), geom.top(), sizeValue, sizeValue);
    QPointF tl = QPointF (b.topLeft().x()+m_barWidth, b.topLeft().y()+m_barWidth);
    QPointF br = QPointF (b.bottomRight().x()-m_barWidth, b.bottomRight().y()-m_barWidth);
    QRect r(tl.toPoint(), br.toPoint());
    return r;
}

QRect KisVisualEllipticalSelectorShape::getSpaceForTriangle(QRect geom)
{
    int sizeValue = qMin(width(),height());
    QRect b(geom.left(), geom.top(), sizeValue, sizeValue);
    QLineF radius(b.center(), QPointF(b.left()+m_barWidth, b.center().y()) );
    radius.setAngle(90);//point at yellowgreen :)
    QPointF t = radius.p2();
    radius.setAngle(330);//point to purple :)
    QPointF br = radius.p2();
    radius.setAngle(210);//point to cerulean :)
    QPointF bl = radius.p2();
    QPointF tl = QPoint(bl.x(),t.y());
    QRect r(tl.toPoint(), br.toPoint());
    return r;
}

QPointF KisVisualEllipticalSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate)
{
    qreal x;
    qreal y;
    qreal offset=7.0;
    qreal a = (qreal)width()*0.5;
    QPointF center(a, a);
    QLineF line(center, QPoint((m_barWidth*0.5),a));
    qreal angle = coordinate.x()*360.0;
    angle = fmod(angle+180.0,360.0);
    angle = 180.0-angle;
    angle = angle+180.0;
    if (m_type==KisVisualEllipticalSelectorShape::borderMirrored) {
        angle = (coordinate.x()/2)*360.0;
        angle = fmod((angle+270.0), 360.0);
    }
    line.setAngle(angle);
    if (getDimensions()!=KisVisualColorSelectorShape::onedimensional) {
        line.setLength(qMin(coordinate.y()*(a-offset), a-offset));
    }
    x = qRound(line.p2().x());
    y = qRound(line.p2().y());
    return QPointF(x,y);
}

QPointF KisVisualEllipticalSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPoint coordinate)
{
    //default implementation:
    qreal x = 0.5;
    qreal y = 1.0;
    qreal offset = 7.0;
    QPointF center = QRectF(QPointF(0.0, 0.0), this->size()).center();
    qreal a = (this->width()/2);
    qreal xRel = center.x()-coordinate.x();
    qreal yRel = center.y()-coordinate.y();
    qreal radius = sqrt(xRel*xRel+yRel*yRel);

    if (m_type!=KisVisualEllipticalSelectorShape::borderMirrored){
        qreal angle = atan2(yRel, xRel);
        angle = kisRadiansToDegrees(angle);
        angle = fmod(angle+360, 360.0);
        x = angle/360.0;
        if (getDimensions()==KisVisualColorSelectorShape::twodimensional) {
            y = qBound(0.0,radius/(a-offset), 1.0);
        }

    } else {
        qreal angle = atan2(xRel, yRel);
        angle = kisRadiansToDegrees(angle);
        angle = fmod(angle+180, 360.0);
        if (angle>180.0) {
            angle = 360.0-angle;
        }
        x = (angle/360.0)*2;
        if (getDimensions()==KisVisualColorSelectorShape::twodimensional) {
            y = qBound(0.0,(radius+offset)/a, 1.0);
        }
    }

    return QPointF(x, y);
}

QRegion KisVisualEllipticalSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(0,0,width(),height(), QRegion::Ellipse);
    if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
        mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2), QRegion::Ellipse));
    }
    return mask;
}

void KisVisualEllipticalSelectorShape::resizeEvent(QResizeEvent *)
{
    //qDebug() << this << "KisVisualEllipticalSelectorShape::resizeEvent";
    forceImageUpdate();
}

void KisVisualEllipticalSelectorShape::drawCursor()
{
    //qDebug() << this << "KisVisualEllipticalSelectorShape::drawCursor: image needs update" << imagesNeedUpdate();
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QImage fullSelector = getImageMap();
    QColor col = getColorFromConverter(getCurrentColor());
    QPainter painter;
    painter.begin(&fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));

    painter.save();
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    QPen pen;
    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawEllipse(QRect(0,0,width(),height()));

    if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
        painter.setBrush(Qt::SolidPattern);
        painter.drawEllipse(innerRect);
    }
    painter.restore();

    QBrush fill;
    fill.setStyle(Qt::SolidPattern);

    int cursorwidth = 5;

    if (m_type==KisVisualEllipticalSelectorShape::borderMirrored) {
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

    } else {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth-1.0, cursorwidth-1.0);
    }
    painter.end();
    setFullImage(fullSelector);
}
