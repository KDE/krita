/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CURVE_RECT_MASK_GENERATOR_P_H
#define KIS_CURVE_RECT_MASK_GENERATOR_P_H

#include <QScopedPointer>

#include "kis_antialiasing_fade_maker.h"
#include "kis_brush_mask_applicator_base.h"

struct Q_DECL_HIDDEN KisCurveRectangleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    Private(const Private &rhs)
        : xcoeff(rhs.xcoeff),
        ycoeff(rhs.ycoeff),
        curveResolution(rhs.curveResolution),
        curveData(rhs.curveData),
        curvePoints(rhs.curvePoints),
        dirty(rhs.dirty),
        fadeMaker(rhs.fadeMaker, *this)
    {
    }

    qreal xcoeff {0.0};
    qreal ycoeff {0.0};
    qreal curveResolution {0.0};
    QVector<qreal> curveData;
    QList<QPointF> curvePoints;
    bool dirty {false};

    KisAntialiasingFadeMaker2D<Private> fadeMaker;
    QScopedPointer<KisBrushMaskApplicatorBase> applicator;

    inline quint8 value(qreal xr, qreal yr) const;
};

#endif // KIS_CURVE_RECT_MASK_GENERATOR_P_H
