/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBEZIERPATCH_H
#define KISBEZIERPATCH_H

#include "kritaglobal_export.h"

#include <QRectF>
#include <array>

class QDebug;

class KRITAGLOBAL_EXPORT KisBezierPatch
{
public:
    enum ControlPointType {
        TL = 0,
        TL_HC,
        TL_VC,
        TR,
        TR_HC,
        TR_VC,
        BL,
        BL_HC,
        BL_VC,
        BR,
        BR_HC,
        BR_VC
    };

    QRectF originalRect;
    std::array<QPointF, 12> points;

    QRectF dstBoundingRect() const;

    QRectF srcBoundingRect() const;

    QPointF localToGlobal(const QPointF &pt) const;
    QPointF globalToLocal(const QPointF &pt) const;

    void sampleRegularGrid(QSize &gridSize,
                           QVector<QPointF> &origPoints,
                           QVector<QPointF> &transfPoints,
                           const QPointF &dstStep) const;

    void sampleRegularGridSVG2(QSize &gridSize,
                               QVector<QPointF> &origPoints,
                               QVector<QPointF> &transfPoints,
                               const QPointF &dstStep) const;
};

KRITAGLOBAL_EXPORT
QDebug operator<<(QDebug dbg, const KisBezierPatch &p);

#endif // KISBEZIERPATCH_H
