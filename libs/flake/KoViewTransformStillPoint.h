/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOVIEWTRANSFORMSTILLPOINT_H
#define KOVIEWTRANSFORMSTILLPOINT_H

#include <kritaflake_export.h>
#include <QPointF>
#include <utility>

class KisCoordinatesConverter;

/**
 * A special class the defines a "still point" for the transformations
 * applied to the canvas. "Still point" is defined as a point that does
 * not move on screen when a transformation happens. For example, when
 * rotating the canvas, the center of rotation does not move, hence
 * it is considered as a "still point".
 *
 * A lot of canvas operations in Krita cannot guarantee that the
 * point will precisely "stand still" during transformation. It may
 * happen that the canvas will be slightly moved to the side to ensure
 * that it is aligned to the hardware pixels.
 *
 * In such cases we should remember what was our original intention in
 * positioning the image on the canvas. We should remember both, the
 * image point and the widget point. Otherwise slight and unpleasant
 * drifts will happen when the user zooms or rotates the image.
 */
class KRITAFLAKE_EXPORT KoViewTransformStillPoint : public std::pair<QPointF, QPointF>
{
public:
    KoViewTransformStillPoint() = default;
    KoViewTransformStillPoint(const QPointF &docPoint, const QPointF &viewPoint);
    KoViewTransformStillPoint(const std::pair<QPointF, QPointF> &rhs);

    KoViewTransformStillPoint(const KoViewTransformStillPoint &rhs) = default;
    KoViewTransformStillPoint(KoViewTransformStillPoint &&rhs) = default;
    KoViewTransformStillPoint& operator=(const KoViewTransformStillPoint &rhs) = default;
    KoViewTransformStillPoint& operator=(KoViewTransformStillPoint &&rhs) = default;

    QPointF docPoint() const;
    QPointF viewPoint() const;
};

KRITAFLAKE_EXPORT QDebug operator<<(QDebug dbg, const KoViewTransformStillPoint &point);

#endif // KOVIEWTRANSFORMSTILLPOINT_H