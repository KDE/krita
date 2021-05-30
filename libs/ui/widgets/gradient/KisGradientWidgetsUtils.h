/*
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_WIDGETS_UTIL_H
#define KIS_GRADIENT_WIDGETS_UTIL_H

#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QColor>

#include <KoAbstractGradient.h>
#include <KoSegmentGradient.h>
#include <kritaui_export.h>

namespace KisGradientWidgetsUtils
{

enum ColorType
{
    None, Foreground, Background, Custom
};

struct StopHandleColor
{
    ColorType type{None};
    QColor color;
};

void KRITAUI_EXPORT paintGradientBox(QPainter &painter, const KoAbstractGradientSP gradient, const QRectF &rect);
void KRITAUI_EXPORT paintStopHandle(QPainter &painter,
                                    const QPointF &position,
                                    const QSizeF &size,
                                    bool isSelected, bool isHovered, bool hasFocus,
                                    const QColor &highlightColor,
                                    const StopHandleColor &color1,
                                    const StopHandleColor &color2 = {});
void KRITAUI_EXPORT paintMidPointHandle(QPainter &painter,
                                        const QPointF &position,
                                        qreal size,
                                        bool isSelected, bool isHovered, bool hasFocus,
                                        const QColor &borderColor,
                                        const QColor &fillColor,
                                        const QColor &highlightColor);
                                        
KisGradientWidgetsUtils::ColorType KRITAUI_EXPORT segmentEndPointTypeToColorType(KoGradientSegmentEndpointType type);
KoGradientSegmentEndpointType KRITAUI_EXPORT colorTypeToSegmentEndPointType(KisGradientWidgetsUtils::ColorType type, bool transparent = false);

}

#endif
