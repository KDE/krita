/*
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainterPath>

#include <KoCheckerBoardPainter.h>

#include "KisGradientWidgetsUtils.h"

namespace KisGradientWidgetsUtils
{

void paintGradientBox(QPainter &painter, const KoAbstractGradientSP gradient, const QRectF &rect)
{
    static KoCheckerBoardPainter checkerBoardPainter(4);
    
    // Background
    checkerBoardPainter.paint(painter, rect, rect.topLeft());
    // Gradient
    QImage image = gradient->generatePreview(rect.width(), rect.height());
    if (!image.isNull()) {
        painter.drawImage(rect.topLeft(), image);
    }
    // Border
    painter.setPen(QColor(0, 0, 0, 192));
    painter.drawRect(rect);
}

void paintStopHandle(QPainter &painter,
                     const QPointF &position,
                     const QSizeF &size,
                     bool isSelected, bool isHovered, bool hasFocus,
                     const QColor &highlightColor,
                     const StopHandleColor &color1,
                     const StopHandleColor &color2)
{
    painter.save();

    QColor borderColor;
    int borderWidth;
    if (isSelected) {
        borderColor = highlightColor;
        borderColor.setAlpha(255);
        borderWidth = hasFocus ? 2 : 1;
    } else {
        if (isHovered) {
            borderColor = highlightColor;
            borderColor.setAlpha(192);
        } else {
            borderColor = QColor(0, 0, 0, 128);
        }
        borderWidth = 1;
    }
    const QPointF alignedPosition(qRound(position.x() + 0.5) - 0.5, position.y());
    const qreal halfWidth = size.width() * 0.5;
    QPainterPath path(alignedPosition);
    path.arcTo(
        alignedPosition.x() - halfWidth,
        alignedPosition.y() + size.height() - size.width(),
        size.width(),
        size.width(),
        150,
        240
    );
    path.closeSubpath();
    // paint the "drop" handle
    if (color1.type != None && color2.type != None && color1.color != color2.color) {
        painter.setClipRect(QRectF(QPointF(alignedPosition.x() - halfWidth, alignedPosition.y()), QSizeF(halfWidth, size.height())));
        painter.setPen(Qt::NoPen);
        painter.setBrush(color1.color);
        painter.drawPath(path);
        painter.setClipRect(QRectF(alignedPosition, QSizeF(halfWidth, size.height())));
        painter.setBrush(color2.color);
        painter.drawPath(path);
        painter.setClipping(false);
        painter.setPen(QPen(borderColor, borderWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    } else {
        painter.setPen(QPen(borderColor, borderWidth));
        if (color1.type != None) {
            // Only color 1 was specified
            painter.setBrush(color1.color);
        } else {
            // Only color 2 was specified
            painter.setBrush(color2.color);
        }
        painter.drawPath(path);
    }
    // paint the type indicator
    constexpr QSizeF typeIndicatorSize(5.0, 5.0);
    if (color1.type == Foreground || color1.type == Background) {
        QPointF typeIndicatorPosition(
            alignedPosition.x() - halfWidth - 1.0,
            alignedPosition.y() + size.height() - typeIndicatorSize.height() + 1.0
        );
        if (color1.type == Foreground) {
            painter.setPen(QPen(Qt::white, 1));
            painter.setBrush(Qt::black);
        } else if (color1.type == Background) {
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
        }
        painter.drawEllipse(QRectF(typeIndicatorPosition, typeIndicatorSize));
    }
    if (color2.type == Foreground || color2.type == Background) {
        QPointF typeIndicatorPosition(
            alignedPosition.x() + halfWidth - typeIndicatorSize.width() + 1.0,
            alignedPosition.y() + size.height() - typeIndicatorSize.height() + 1.0
        );
        if (color2.type == Foreground) {
            painter.setPen(QPen(Qt::white, 1));
            painter.setBrush(Qt::black);
        } else if (color2.type == Background) {
            painter.setPen(QPen(Qt::black, 1));
            painter.setBrush(Qt::white);
        }
        painter.drawEllipse(QRectF(typeIndicatorPosition, typeIndicatorSize));
    }

    painter.restore();
}

void paintMidPointHandle(QPainter &painter,
                         const QPointF &position,
                         qreal size,
                         bool isSelected, bool isHovered, bool hasFocus,
                         const QColor &borderColor,
                         const QColor &fillColor,
                         const QColor &highlightColor)
{
    painter.save();

    QColor brushColor;
    int penWidth;
    if (isSelected) {
        brushColor = highlightColor;
        penWidth = hasFocus ? 2 : 1;
    } else {
        if (isHovered) {
            brushColor = highlightColor;
            brushColor.setAlpha(192);
        } else {
            brushColor = fillColor;
        }
        penWidth = 1;
    }
    const QPointF alignedPosition(qRound(position.x() + 0.5) - 0.5, qRound(position.y() + 0.5) - 0.5);
    const qreal alignedSize = qRound(size);
    const qreal handleSizeOverTwo = alignedSize * 0.5;
    const QPointF points[4] =
        {
            QPointF(0.0, 0.0),
            QPointF(-handleSizeOverTwo, handleSizeOverTwo),
            QPointF(0.0, size),
            QPointF(handleSizeOverTwo, handleSizeOverTwo)
        };
    painter.translate(alignedPosition);
    painter.setPen(QPen(borderColor, penWidth));
    painter.setBrush(brushColor);
    painter.drawPolygon(points, 4);

    painter.restore();
}


KisGradientWidgetsUtils::ColorType segmentEndPointTypeToColorType(KoGradientSegmentEndpointType type)
{
    if (type == FOREGROUND_ENDPOINT || type == FOREGROUND_TRANSPARENT_ENDPOINT) {
        return KisGradientWidgetsUtils::Foreground;
    } else if (type == BACKGROUND_ENDPOINT || type == BACKGROUND_TRANSPARENT_ENDPOINT) {
        return KisGradientWidgetsUtils::Background;
    }
    return KisGradientWidgetsUtils::Custom;
}

KoGradientSegmentEndpointType colorTypeToSegmentEndPointType(KisGradientWidgetsUtils::ColorType type, bool transparent)
{
    if (type == Foreground) {
        return transparent ? FOREGROUND_TRANSPARENT_ENDPOINT : FOREGROUND_ENDPOINT;
    } else if (type == Background) {
        return transparent ? BACKGROUND_TRANSPARENT_ENDPOINT : BACKGROUND_ENDPOINT;
    }
    return COLOR_ENDPOINT;
}

}
