/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualRectangleSelectorShape.h"

#include <QColor>
#include <QPainter>
#include <QRect>
#include <QList>
#include <QLineF>
#include <QtMath>

#include "kis_debug.h"

KisVisualRectangleSelectorShape::KisVisualRectangleSelectorShape(KisVisualColorSelector *parent,
                                                                 Dimensions dimension,
                                                                 int channel1, int channel2,
                                                                 int width,
                                                                 singelDTypes d)
    : KisVisualColorSelectorShape(parent, dimension, channel1, channel2)
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

void KisVisualRectangleSelectorShape::setOneDimensionalType(KisVisualRectangleSelectorShape::singelDTypes type)
{
    if (type != m_type) {
        m_type = type;
        forceImageUpdate();
        update();
    }
}

QRect KisVisualRectangleSelectorShape::getSpaceForSquare(QRect geom)
{
    return getAvailableSpace(geom, true);
}

QRect KisVisualRectangleSelectorShape::getSpaceForCircle(QRect geom)
{
    return getAvailableSpace(geom, false);
}

QRect KisVisualRectangleSelectorShape::getSpaceForTriangle(QRect geom)
{
    return getSpaceForSquare(geom);
}

QRect KisVisualRectangleSelectorShape::getAvailableSpace(QRect geom, bool stretch)
{
    QPoint tl;
    QPoint br;

    if (m_type == KisVisualRectangleSelectorShape::vertical) {
        br = geom.bottomRight();
        tl = QPoint(geom.topLeft().x() + m_barWidth, geom.topLeft().y());
    } else if (m_type == KisVisualRectangleSelectorShape::horizontal) {
        br = geom.bottomRight();
        tl = QPoint(geom.topLeft().x(), geom.topLeft().y() + m_barWidth);
    } else {
        tl = QPoint(geom.topLeft().x() + m_barWidth, geom.topLeft().y() + m_barWidth);
        br = QPoint(geom.bottomRight().x() - m_barWidth, geom.bottomRight().y() - m_barWidth);
    }
    QRect available(tl, br);
    if (!stretch) {
        int size = qMin(available.height(), available.width());
        if (m_type == KisVisualRectangleSelectorShape::vertical) {
            available.moveTop(available.y() + (available.height() - size)/2);
        } else if (m_type == KisVisualRectangleSelectorShape::horizontal) {
            available.moveLeft(available.x() + (available.width() - size)/2);
        }
        available.setSize(QSize(size, size));
    }

    return available;
}

QPointF KisVisualRectangleSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const
{
    // Reminder: in Qt widget space, origin is top-left, but we want zero y to be at the bottom
    qreal x = 0.5 * width();
    qreal y = 0.5 * height();
    qreal offset = 5.0;
    KisVisualColorSelectorShape::Dimensions dimension = getDimensions();
    if (dimension == KisVisualColorSelectorShape::onedimensional) {
        if ( m_type == KisVisualRectangleSelectorShape::vertical) {
            y = qMin((1.0 - coordinate.x())*(height()-offset*2)+offset, (qreal)height());
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
        x = qMin(coordinate.x()*(width()-offset*2)+offset, (qreal)width());
        y = qMin((1.0 - coordinate.y())*(height()-offset*2)+offset, (qreal)height());
    }
    return QPointF(x,y);
}

QPointF KisVisualRectangleSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const
{
    // Reminder: in Qt widget space, origin is top-left, but we want zero y to be at the bottom
    //default values:
    qreal x = 0.5;
    qreal y = 0.5;
    qreal offset = 5.0;
    KisVisualColorSelectorShape::Dimensions dimension = getDimensions();
    if (dimension == KisVisualColorSelectorShape::onedimensional ) {
        if (m_type == KisVisualRectangleSelectorShape::vertical) {
            x = 1.0 - (coordinate.y()-offset)/(height()-offset*2);
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
        y = 1.0 - (coordinate.y()-offset)/(height()-offset*2);
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

QImage KisVisualRectangleSelectorShape::renderAlphaMask() const
{
    // Hi-DPI aware rendering requires that we determine the device pixel dimension;
    // actual widget size in device pixels is not accessible unfortunately, it might be 1px smaller...
    const int deviceWidth = qCeil(width() * devicePixelRatioF());
    const int deviceHeight = qCeil(height() * devicePixelRatioF());

    QImage alphaMask(deviceWidth, deviceHeight, QImage::Format_Alpha8);
    alphaMask.fill(0);
    alphaMask.setDevicePixelRatio(devicePixelRatioF());
    QPainter painter(&alphaMask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);

    if (getDimensions() == onedimensional) {
        if (m_type == KisVisualRectangleSelectorShape::vertical) {
            painter.drawRect(2, 3, width() - 4, height() - 6);
        } else if (m_type == KisVisualRectangleSelectorShape::horizontal) {
            painter.drawRect(3, 2, width() - 6, height() - 4);
        } else /*if (m_type == border || m_type == borderMirrored)*/ {
            painter.drawRect(2, 2, width() - 4, height() - 4);
            painter.setBrush(Qt::black);
            painter.drawRect(m_barWidth, m_barWidth, width() - 2 * m_barWidth, height() - 2 * m_barWidth);
        }
    } else {
        painter.drawRect(3, 3, width() - 6, height() - 6);
    }

    return alphaMask;
}

void KisVisualRectangleSelectorShape::drawCursor(QPainter &painter)
{
    //qDebug() << this << "KisVisualRectangleSelectorShape::drawCursor: image needs update" << imagesNeedUpdate();
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QColor col = getColorFromConverter(getCurrentColor());
    QBrush fill(Qt::SolidPattern);

    int cursorwidth = 5;

    if (getDimensions() == KisVisualColorSelectorShape::onedimensional
            && m_type != KisVisualRectangleSelectorShape::border
            && m_type != KisVisualRectangleSelectorShape::borderMirrored) {
        QRectF rect;
        if (m_type==KisVisualRectangleSelectorShape::vertical) {
            qreal y = qRound(cursorPoint.y());
            rect.setCoords(1.5, y - cursorwidth + 0.5, width() - 1.5, y + cursorwidth - 0.5);
        } else {
            qreal x = qRound(cursorPoint.x());
            rect.setCoords(x - cursorwidth  + 0.5, 1.5, x + cursorwidth - 0.5, height() - 1.5);
        }
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawRect(rect);
        fill.setColor(col);
        painter.setPen(Qt::black);
        painter.setBrush(fill);
        painter.drawRect(rect.adjusted(1, 1, -1, -1));

    }else if(m_type==KisVisualRectangleSelectorShape::borderMirrored){
        QRectF innerRect(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2));
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
}
