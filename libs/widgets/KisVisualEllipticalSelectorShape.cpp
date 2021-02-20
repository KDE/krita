/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualEllipticalSelectorShape.h"

#include <QColor>
#include <QPainter>
#include <QRect>
#include <QLineF>
#include <QtMath>

#include "kis_debug.h"
#include "kis_global.h"

KisVisualEllipticalSelectorShape::KisVisualEllipticalSelectorShape(QWidget *parent,
                                                                 Dimensions dimension,
                                                                 const KoColorSpace *cs,
                                                                 int channel1, int channel2,
                                                                 const KoColorDisplayRendererInterface *displayRenderer,
                                                                 int barWidth,
                                                                 singelDTypes d)
    : KisVisualColorSelectorShape(parent, dimension, cs, channel1, channel2, displayRenderer)
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
    forceImageUpdate();
    update();
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
    QPointF center(0.5 * width(), 0.5 * height());
    qreal radius = 0.5 * sizeValue - (m_barWidth + 4);
    QLineF rLine(center, QPointF(center.x() + radius, center.y()));
    rLine.setAngle(330);
    QPoint br(rLine.p2().toPoint());
    //QPoint br(qCeil(rLine.p2().x()), qCeil(rLine.p2().y()));
    QPoint tl(width() - br.x(), m_barWidth + 4);
    QRect bound(tl, br);
    // adjust with triangle default margin for cursor rendering
    // it's not +5 because above calculation is for pixel center and ignores
    // the fact that dimensions are then effectively 1px smaller...
    bound.adjust(-5, -5, 4, 4);
    return bound.intersected(geom);
}

QPointF KisVisualEllipticalSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const
{
    qreal offset = 7.0;
    qreal a = (qreal)width()*0.5;
    QPointF center(a, a);
    QLineF line(center, QPoint((m_barWidth*0.5),a));
    qreal angle = coordinate.x()*360.0;
    angle = 360.0 - fmod(angle+180.0, 360.0);
    if (m_type==KisVisualEllipticalSelectorShape::borderMirrored) {
        angle = (coordinate.x()/2)*360.0;
        angle = fmod((angle+270.0), 360.0);
    }
    line.setAngle(angle);
    if (getDimensions()!=KisVisualColorSelectorShape::onedimensional) {
        line.setLength(qMin(coordinate.y()*(a-offset), a-offset));
    }
    return line.p2();
}

QPointF KisVisualEllipticalSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const
{
    //default implementation:
    qreal x = 0.5;
    qreal y = 1.0;
    qreal offset = 7.0;
    QPointF center = QRectF(QPointF(0.0, 0.0), this->size()).center();
    qreal a = (qreal(this->width()) / qreal(2));
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

QPointF KisVisualEllipticalSelectorShape::mousePositionToShapeCoordinate(const QPointF &pos, const QPointF &dragStart) const
{
    QPointF pos2(pos);
    if (m_type == KisVisualEllipticalSelectorShape::borderMirrored) {
        qreal h_center = width()/2.0;
        bool start_left = dragStart.x() < h_center;
        bool cursor_left = pos.x() < h_center;
        if (start_left != cursor_left) {
            pos2.setX(h_center);
        }
    }
    return convertWidgetCoordinateToShapeCoordinate(pos2);
}

QRegion KisVisualEllipticalSelectorShape::getMaskMap()
{
    QRegion mask = QRegion(0,0,width(),height(), QRegion::Ellipse);
    if (getDimensions()==KisVisualColorSelectorShape::onedimensional) {
        mask = mask.subtracted(QRegion(m_barWidth, m_barWidth, width()-(m_barWidth*2), height()-(m_barWidth*2), QRegion::Ellipse));
    }
    return mask;
}

QImage KisVisualEllipticalSelectorShape::renderAlphaMask() const
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
    painter.drawEllipse(2, 2, width() - 4, height() - 4);
    //painter.setBrush(Qt::black);
    if (getDimensions() == KisVisualColorSelectorShape::onedimensional) {
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.drawEllipse(m_barWidth - 2, m_barWidth - 2, width() - 2*(m_barWidth-2), height() - 2*(m_barWidth-2));
    }
    return alphaMask;
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
    QBrush fill;
    fill.setStyle(Qt::SolidPattern);

    int cursorwidth = 5;

    if (m_type==KisVisualEllipticalSelectorShape::borderMirrored) {
        painter.setPen(Qt::white);
        fill.setColor(Qt::white);
        painter.setBrush(fill);
        painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
        QPointF mirror(width() - cursorPoint.x(), cursorPoint.y());
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
