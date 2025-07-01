/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CANVAS_STATE_H
#define KIS_CANVAS_STATE_H

#include <boost/operators.hpp>
#include <QPoint>
#include <QSize>
#include <QRectF>
#include <KoZoomMode.h>

class KoZoomState;
class KisCoordinatesConverter;

class KisCanvasState : public boost::equality_comparable<KisCanvasState>
{
public:
    qreal zoom;
    qreal effectiveZoom;
    KoZoomMode::Mode zoomMode;
    qreal rotation;
    bool mirrorHorizontally;
    bool mirrorVertically;
    QPoint documentOffset;
    QPointF documentOffsetF;
    QPointF viewportOffsetF;
    QPoint minimumOffset;
    QPoint maximumOffset;
    QSizeF canvasSize;
    qreal minimumZoom;
    qreal maximumZoom;
    QRectF imageRectInWidgetPixels;

    bool operator==(const KisCanvasState &other) const;

    static KisCanvasState fromConverter(const KisCoordinatesConverter &converter);

    KoZoomState zoomState() const;
};

#endif // KIS_CANVAS_STATE_H