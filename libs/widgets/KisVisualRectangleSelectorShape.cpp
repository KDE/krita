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
#include "KisVisualRectangleSelectorShape.h"

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

KisVisualRectangleSelectorShape::KisVisualRectangleSelectorShape(QWidget *parent,
                                                                 Dimensions dimension,
                                                                 ColorModel model,
                                                                 const KoColorSpace *cs,
                                                                 int channel1, int channel2,
                                                                 const KoColorDisplayRendererInterface *displayRenderer,
                                                                 int width,
                                                                 singelDTypes d)
    : KisVisualColorSelectorShape(parent, dimension, model, cs, channel1, channel2, displayRenderer)
{
    //qDebug()  << "creating KisVisualRectangleSelectorShape" << this;
    m_type = d;
    m_barWidth = width;
}

KisVisualRectangleSelectorShape::~KisVisualRectangleSelectorShape()
{
    //qDebug() << "deleting KisVisualRectangleSelectorShape" << this;
}

void KisVisualRectangleSelectorShape::setBorderWidth(int width)
{
    m_barWidth = width;
}

QRect KisVisualRectangleSelectorShape::getSpaceForSquare(QRect geom)
{
    QPointF tl;
    QPointF br;

    if (m_type==KisVisualRectangleSelectorShape::vertical) {
        br = geom.bottomRight();
        tl = QPoint(geom.topLeft().x()+m_barWidth, geom.topLeft().y());
    } else if (m_type==KisVisualRectangleSelectorShape::horizontal) {
        br = geom.bottomRight();
        tl = QPoint(geom.topLeft().x(), geom.topLeft().y()+m_barWidth);
    } else {
        tl = QPointF (geom.topLeft().x()+m_barWidth, geom.topLeft().y()+m_barWidth);
        br = QPointF (geom.bottomRight().x()-m_barWidth, geom.bottomRight().y()-m_barWidth);

    }
    QRect a(tl.toPoint(), br.toPoint());
    QRect r(a.left(), a.top(), qMin(a.height(), a.width()), qMin(a.height(), a.width()));
    return r;
}

QRect KisVisualRectangleSelectorShape::getSpaceForCircle(QRect geom)
{
    return getSpaceForSquare(geom);
}

QRect KisVisualRectangleSelectorShape::getSpaceForTriangle(QRect geom)
{
    return getSpaceForSquare(geom);
}

QPointF KisVisualRectangleSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const
{
    qreal x = 0.5 * m_barWidth;
    qreal y = 0.5 * m_barWidth;
    qreal offset = 5.0;
    KisVisualColorSelectorShape::Dimensions dimension = getDimensions();
    if (dimension == KisVisualColorSelectorShape::onedimensional) {
        if ( m_type == KisVisualRectangleSelectorShape::vertical) {
            y = qMin(coordinate.x()*(height()-offset*2)+offset, (qreal)height());
        } else if (m_type == KisVisualRectangleSelectorShape::horizontal) {
            x = qMin(coordinate.x()*(width()-offset*2)+offset, (qreal)width());
        } else if (m_type == KisVisualRectangleSelectorShape::border) {

            QRectF innerRect(m_barWidth/2, m_barWidth/2, width()-m_barWidth, height()-m_barWidth);
            QPointF left (innerRect.left(),innerRect.center().y());
            QList <QLineF> polygonLines;
            polygonLines.append(QLineF(left, innerRect.topLeft()));
            polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
            polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
            polygonLines.append(QLineF(innerRect.bottomRight(), innerRect.bottomLeft()));
            polygonLines.append(QLineF(innerRect.bottomLeft(), left));

            qreal totalLength =0.0;
            Q_FOREACH(QLineF line, polygonLines) {
                totalLength += line.length();
            }

            qreal length = coordinate.x()*totalLength;
            QPointF intersect(x,y);
            Q_FOREACH(QLineF line, polygonLines) {
                if (line.length()>length && length>0){
                    intersect = line.pointAt(length/line.length());

                }
                length-=line.length();
            }
            x = qRound(intersect.x());
            y = qRound(intersect.y());

        }
        else /*if (m_type == KisVisualRectangleSelectorShape::borderMirrored)*/  {

            QRectF innerRect(m_barWidth/2, m_barWidth/2, width()-m_barWidth, height()-m_barWidth);
            QPointF bottom (innerRect.center().x(), innerRect.bottom());
            QList <QLineF> polygonLines;
            polygonLines.append(QLineF(bottom, innerRect.bottomLeft()));
            polygonLines.append(QLineF(innerRect.bottomLeft(), innerRect.topLeft()));
            polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
            polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
            polygonLines.append(QLineF(innerRect.bottomRight(), bottom));

            qreal totalLength =0.0;
            Q_FOREACH(QLineF line, polygonLines) {
                totalLength += line.length();
            }

            qreal length = coordinate.x()*(totalLength/2);
            QPointF intersect(x,y);
            if (coordinate.y()==1) {
                for (int i = polygonLines.size()-1; i==0; i--) {
                    QLineF line = polygonLines.at(i);
                    if (line.length()>length && length>0){
                        intersect = line.pointAt(length/line.length());

                    }
                    length-=line.length();
                }
            } else {
                Q_FOREACH(QLineF line, polygonLines) {
                    if (line.length()>length && length>0){
                        intersect = line.pointAt(length/line.length());

                    }
                    length-=line.length();
                }
            }
            x = qRound(intersect.x());
            y = qRound(intersect.y());

        }
    } else {
        x = qMin(coordinate.x()*(height()-offset*2)+offset, (qreal)height());
        y = qMin(coordinate.y()*(width()-offset*2)+offset, (qreal)width());
    }
    return QPointF(x,y);
}

QPointF KisVisualRectangleSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPoint coordinate) const
{
    //default implementation:
    qreal x = 0.5;
    qreal y = 0.5;
    qreal offset = 5.0;
    KisVisualColorSelectorShape::Dimensions dimension = getDimensions();
    if (dimension == KisVisualColorSelectorShape::onedimensional ) {
        if (m_type == KisVisualRectangleSelectorShape::vertical) {
            x = (coordinate.y()-offset)/(height()-offset*2);
        } else if (m_type == KisVisualRectangleSelectorShape::horizontal) {
            x = (coordinate.x()-offset)/(width()-offset*2);
        } else if (m_type == KisVisualRectangleSelectorShape::border) {
            //border

            QRectF innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
            QPointF left (innerRect.left(),innerRect.center().y());
            QList <QLineF> polygonLines;
            polygonLines.append(QLineF(left, innerRect.topLeft()));
            polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
            polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
            polygonLines.append(QLineF(innerRect.bottomRight(), innerRect.bottomLeft()));
            polygonLines.append(QLineF(innerRect.bottomLeft(), left));

            QLineF radius(coordinate, this->geometry().center());
            QPointF intersect(0.5,0.5);
            qreal length = 0.0;
            qreal totalLength = 0.0;
            bool foundIntersect = false;
            Q_FOREACH(QLineF line, polygonLines) {
                if (line.intersect(radius,&intersect)==QLineF::BoundedIntersection && foundIntersect==false)
                {
                    foundIntersect = true;
                    length+=QLineF(line.p1(), intersect).length();

                }
                if (foundIntersect==false) {
                    length+=line.length();
                }
                totalLength+=line.length();
            }

            x = length/totalLength;

        } else /*if (m_type == KisVisualRectangleSelectorShape::borderMirrored)*/  {
            //border

            QRectF innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
            QPointF bottom (innerRect.center().x(), innerRect.bottom());
            QList <QLineF> polygonLines;
            polygonLines.append(QLineF(bottom, innerRect.bottomLeft()));
            polygonLines.append(QLineF(innerRect.bottomLeft(), innerRect.topLeft()));
            polygonLines.append(QLineF(innerRect.topLeft(), innerRect.topRight()));
            polygonLines.append(QLineF(innerRect.topRight(), innerRect.bottomRight()));
            polygonLines.append(QLineF(innerRect.bottomRight(), bottom));

            QLineF radius(coordinate, this->geometry().center());
            QPointF intersect(0.5,0.5);
            qreal length = 0.0;
            qreal totalLength = 0.0;
            bool foundIntersect = false;
            Q_FOREACH(QLineF line, polygonLines) {
                if (line.intersect(radius,&intersect)==QLineF::BoundedIntersection && foundIntersect==false)
                {
                    foundIntersect = true;
                    length+=QLineF(line.p1(), intersect).length();

                }
                if (foundIntersect==false) {
                    length+=line.length();
                }
                totalLength+=line.length();
            }
            int halflength = totalLength/2;

            if (length>halflength) {
                x = (halflength - (length-halflength))/halflength;
                y = 1.0;
            } else {
                x = length/halflength;
                y = 0.0;
            }
        }
    }
    else {
        x = (coordinate.x()-offset)/(width()-offset*2);
        y = (coordinate.y()-offset)/(height()-offset*2);
    }
    x = qBound((qreal)0.0, x, (qreal)1.0);
    y = qBound((qreal)0.0, y, (qreal)1.0);
    return QPointF(x, y);
}

QRegion KisVisualRectangleSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(0,0,width(),height());
    if (m_type==KisVisualRectangleSelectorShape::border || m_type==KisVisualRectangleSelectorShape::borderMirrored) {
        mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2)));
    }
    return mask;
}
void KisVisualRectangleSelectorShape::resizeEvent(QResizeEvent *)
{
    //qDebug()  << this << "KisVisualRectangleSelectorShape::resizeEvent";
    forceImageUpdate();
}

void KisVisualRectangleSelectorShape::drawCursor()
{
    //qDebug() << this << "KisVisualRectangleSelectorShape::drawCursor: image needs update" << imagesNeedUpdate();
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QImage fullSelector = getImageMap();
    QColor col = getColorFromConverter(getCurrentColor());
    QPainter painter;
    painter.begin(&fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);
    //QPainterPath path;
    QBrush fill;
    fill.setStyle(Qt::SolidPattern);

    int cursorwidth = 5;
    QRect rect(cursorPoint.toPoint().x()-cursorwidth,cursorPoint.toPoint().y()-cursorwidth,
               cursorwidth*2,cursorwidth*2);
    if (m_type==KisVisualRectangleSelectorShape::vertical){
        int x = ( cursorPoint.x()-(width()/2)+1 );
        int y = ( cursorPoint.y()-cursorwidth );
        rect.setCoords(x, y, x+width()-2, y+(cursorwidth*2));
        painter.save();
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        QPen pen;
        pen.setWidth(5);
        painter.setPen(pen);
        painter.drawLine(QLine(QPoint(0.0,0.0), QPoint(0.0,height())));
        painter.drawLine(QLine(QPoint(width(),0.0), QPoint(width(),height())));
        painter.restore();
    } else {
        int x = cursorPoint.x()-cursorwidth;
        int y = cursorPoint.y()-(height()/2)+1;
        rect.setCoords(x, y, x+(cursorwidth*2), y+cursorwidth-2);
    }
    QRectF innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
    if (getDimensions() == KisVisualColorSelectorShape::onedimensional && m_type!=KisVisualRectangleSelectorShape::border && m_type!=KisVisualRectangleSelectorShape::borderMirrored) {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawRect(rect);
        //set filter conversion!
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        rect.setCoords(rect.topLeft().x()+1, rect.topLeft().y()+1,
                       rect.topLeft().x()+rect.width()-2, rect.topLeft().y()+rect.height()-2);
        painter.drawRect(rect);

    }else if(m_type==KisVisualRectangleSelectorShape::borderMirrored){
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
        painter.save();
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        QPen pen;
        pen.setWidth(5);
        painter.setPen(pen);
        painter.drawRect(QRect(0,0,width(),height()));
        painter.restore();

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
