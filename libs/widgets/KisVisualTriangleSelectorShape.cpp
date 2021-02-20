/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualTriangleSelectorShape.h"

#include <QColor>
#include <QPainter>
#include <QRect>
#include <QtMath>

#include "kis_debug.h"
#include "kis_global.h"

KisVisualTriangleSelectorShape::KisVisualTriangleSelectorShape(QWidget *parent,
                                                               Dimensions dimension,
                                                               const KoColorSpace *cs,
                                                               int channel1, int channel2,
                                                               const KoColorDisplayRendererInterface *displayRenderer,
                                                               int margin)
    : KisVisualColorSelectorShape(parent, dimension, cs, channel1, channel2, displayRenderer),
      m_margin(margin)
{
    //qDebug() << "creating KisVisualTriangleSelectorShape" << this;
}

KisVisualTriangleSelectorShape::~KisVisualTriangleSelectorShape()
{
    //qDebug() << "deleting KisVisualTriangleSelectorShape" << this;
}

void KisVisualTriangleSelectorShape::setBorderWidth(int /*width*/)
{
    // triangle doesn't have a 1-dimensional mode
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

QPointF KisVisualTriangleSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const
{
    // margin serves to render the cursor, and triangle is rendered 1px larger than its active area
    qreal offset = m_margin + 1.0;

    qreal y = (coordinate.y() * (height() - 1 - 2 * offset)) + offset;

    qreal triWidth = width() - 1 - 2 * offset;
    qreal horizontalLineLength = coordinate.y() * triWidth;
    qreal horizontalLineStart = offset + 0.5 * (triWidth - horizontalLineLength);

    qreal x = coordinate.x() * horizontalLineLength + horizontalLineStart;

    return QPointF(x, y);
}

QPointF KisVisualTriangleSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const
{
    // margin serves to render the cursor, and triangle is rendered 1px larger than its active area
    qreal offset = m_margin + 1.0;

    qreal x = 0.5;
    qreal y = qBound(0.0, (coordinate.y() - offset)/(height() - 1 - 2 * offset), 1.0);

    if (y > 0) {
        qreal triWidth = width() - 1 - 2 * offset;
        qreal horizontalLineLength = y * triWidth;
        qreal horizontalLineStart = offset + 0.5 * (triWidth - horizontalLineLength);

        x = qBound(0.0, (coordinate.x() - horizontalLineStart) / horizontalLineLength, 1.0);
    }

    return QPointF(x, y);
}

QRegion KisVisualTriangleSelectorShape::getMaskMap()
{
    const int cursorWidth = qMax(2 * m_margin, 2);
    QPolygon maskPoly;
    maskPoly << QPoint(qFloor(0.5 * (width() - cursorWidth)), 0)
             << QPoint(qCeil(0.5 * (width() + cursorWidth)), 0)
             << QPoint(width(), height() - cursorWidth)
             << QPoint(width(), height())
             << QPoint(0, height())
             << QPoint(0, height() - cursorWidth);

    return QRegion(maskPoly);
}

QImage KisVisualTriangleSelectorShape::renderAlphaMask() const
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
    QPointF triangle[3] = {
        QPointF(0.5 * width(), m_margin),
        QPointF(m_margin, height() - m_margin),
        QPointF(width() - m_margin, height() - m_margin),
    };
    painter.drawConvexPolygon(triangle, 3);

    return alphaMask;
}

void KisVisualTriangleSelectorShape::drawCursor()
{
    //qDebug() << this << "KisVisualTriangleSelectorShape::drawCursor: image needs update" << imagesNeedUpdate();
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QImage fullSelector = getImageMap();
    QColor col = getColorFromConverter(getCurrentColor());
    QPainter painter(&fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);

    QBrush fill(Qt::SolidPattern);

    int cursorwidth = 5;

    painter.setPen(Qt::white);
    fill.setColor(Qt::white);
    painter.setBrush(fill);
    painter.drawEllipse(cursorPoint, cursorwidth, cursorwidth);
    fill.setColor(col);
    painter.setPen(Qt::black);
    painter.setBrush(fill);
    painter.drawEllipse(cursorPoint, cursorwidth-1.0, cursorwidth-1.0);

    painter.end();
    setFullImage(fullSelector);
}
