/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CURVE_CIRCLE_MASK_GENERATOR_P_H
#define KIS_CURVE_CIRCLE_MASK_GENERATOR_P_H

#include "kis_antialiasing_fade_maker.h"
#include "kis_brush_mask_applicator_base.h"

struct Q_DECL_HIDDEN KisCurveCircleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    Private(const Private &rhs)
        : xcoef(rhs.xcoef),
        ycoef(rhs.ycoef),
        curveResolution(rhs.curveResolution),
        curveData(rhs.curveData),
        curvePoints(rhs.curvePoints),
        dirty(true),
        fadeMaker(rhs.fadeMaker,*this)
    {
    }

    qreal xcoef {0.0};
    qreal ycoef {0.0};
    qreal curveResolution {0.0};
    QVector<qreal> curveData;
    QList<QPointF> curvePoints;
    bool dirty {false};

    KisAntialiasingFadeMaker1D<Private> fadeMaker;
    QScopedPointer<KisBrushMaskApplicatorBase> applicator;

    inline quint8 value(qreal dist) const;
};

#endif // KIS_CURVE_CIRCLE_MASK_GENERATOR_P_H
