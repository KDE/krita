/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GAUSS_RECT_MASK_GENERATOR_P_H
#define KIS_GAUSS_RECT_MASK_GENERATOR_P_H

#include <QScopedPointer>

#include "kis_antialiasing_fade_maker.h"
#include "kis_brush_mask_applicator_base.h"

struct Q_DECL_HIDDEN KisGaussRectangleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    Private(const Private &rhs)
        : xfade(rhs.xfade),
        yfade(rhs.yfade),
        halfWidth(rhs.halfWidth),
        halfHeight(rhs.halfHeight),
        alphafactor(rhs.alphafactor),
        fadeMaker(rhs.fadeMaker, *this)
    {
    }

    qreal xfade {0.0};
    qreal yfade {0.0};
    qreal halfWidth, halfHeight;
    qreal alphafactor {0.0};

    KisAntialiasingFadeMaker2D <Private> fadeMaker;

    QScopedPointer<KisBrushMaskApplicatorBase> applicator;

    inline quint8 value(qreal x, qreal y) const;

};

#endif // KIS_GAUSS_RECT_MASK_GENERATOR_P_H
