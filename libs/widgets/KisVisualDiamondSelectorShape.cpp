/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisVisualDiamondSelectorShape.h"

#include <QPainter>
#include <QRect>
#include <QtMath>

#include "kis_debug.h"
#include "kis_global.h"

KisVisualDiamondSelectorShape::KisVisualDiamondSelectorShape(KisVisualColorSelector *parent,
                                                               Dimensions dimension,
                                                               int channel1, int channel2,
                                                               int margin)
    : KisVisualColorSelectorShape(parent, dimension, channel1, channel2),
      m_margin(margin)
{
}

KisVisualDiamondSelectorShape::~KisVisualDiamondSelectorShape()
{
}

void KisVisualDiamondSelectorShape::setBorderWidth(int /*width*/)
{
    // Diamond doesn't have a 1-dimensional mode
}

QRect KisVisualDiamondSelectorShape::getSpaceForSquare(QRect geom)
{
    return geom;
}

QRect KisVisualDiamondSelectorShape::getSpaceForCircle(QRect geom)
{
    return geom;
}

QRect KisVisualDiamondSelectorShape::getSpaceForTriangle(QRect geom)
{
    return geom;
}

QPointF KisVisualDiamondSelectorShape::convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const
{
    // margin serves to render the cursor, and triangle is rendered 1px larger than its active area
    qreal offset = m_margin + 1.0;

    qreal y = ((1.0 - coordinate.y()) * (height() - 1 - 2 * offset)) + offset;

    qreal triWidth = width() - 1 - 2 * offset;
    qreal horizontalLineLength;
    if (coordinate.y() < 0.5) {
        horizontalLineLength = 2.0 * coordinate.y() * triWidth;
    } else {
        horizontalLineLength = 2.0 * (1.0 - coordinate.y()) * triWidth;
    }
    qreal horizontalLineStart = offset + 0.5 * (triWidth - horizontalLineLength);

    qreal x = coordinate.x() * horizontalLineLength + horizontalLineStart;

    return QPointF(x, y);
}

QPointF KisVisualDiamondSelectorShape::convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const
{
    // margin serves to render the cursor, and triangle is rendered 1px larger than its active area
    qreal offset = m_margin + 1.0;

    qreal x = 0.5;
    qreal y = qBound(0.0, 1.0 - (coordinate.y() - offset)/(height() - 1 - 2 * offset), 1.0);

    qreal triWidth = width() - 1 - 2 * offset;
    qreal horizontalLineLength;

    if (y < 0.5) {
        horizontalLineLength = 2.0 * y * triWidth;
    } else {
        horizontalLineLength = 2.0 * (1.0 - y) * triWidth;
    }

    if (horizontalLineLength > 1e-4) {
        qreal horizontalLineStart = offset + 0.5 * (triWidth - horizontalLineLength);
        x = qBound(0.0, (coordinate.x() - horizontalLineStart) / horizontalLineLength, 1.0);
    }

    return QPointF(x, y);
}

QRegion KisVisualDiamondSelectorShape::getMaskMap()
{
    const int cursorWidth = qMax(2 * m_margin, 2);
    QPolygon maskPoly;
    maskPoly << QPoint(qFloor(0.5 * (width() - cursorWidth)), 0)
             << QPoint(qCeil(0.5 * (width() + cursorWidth)), 0)
             << QPoint(width(), qFloor(0.5 * height() - cursorWidth))
             << QPoint(width(), qCeil(0.5 * height() + cursorWidth))
             << QPoint(qCeil(0.5 * (width() + cursorWidth)), height())
             << QPoint(qFloor(0.5 * (width() - cursorWidth)), height())
             << QPoint(0, qCeil(0.5 * height() + cursorWidth))
             << QPoint(0, qFloor(0.5 * height() - cursorWidth));

    return QRegion(maskPoly);
}

QImage KisVisualDiamondSelectorShape::renderAlphaMask() const
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
    QPointF diamond[4] = {
        QPointF(0.5 * width(), m_margin),
        QPointF(m_margin, 0.5 * height()),
        QPointF(0.5 * width(), height() - m_margin),
        QPointF(width() - m_margin, 0.5 * height())
    };
    painter.drawConvexPolygon(diamond, 4);

    return alphaMask;
}

void KisVisualDiamondSelectorShape::drawCursor(QPainter &painter)
{
    QPointF cursorPoint = convertShapeCoordinateToWidgetCoordinate(getCursorPosition());
    QColor col = getColorFromConverter(getCurrentColor());
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
}
