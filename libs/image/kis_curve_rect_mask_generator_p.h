/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    qreal xcoeff, ycoeff;
    qreal curveResolution;
    QVector<qreal> curveData;
    QList<QPointF> curvePoints;
    bool dirty;

    KisAntialiasingFadeMaker2D<Private> fadeMaker;
    QScopedPointer<KisBrushMaskApplicatorBase> applicator;

    inline quint8 value(qreal xr, qreal yr) const;
};

#endif // KIS_CURVE_RECT_MASK_GENERATOR_P_H
