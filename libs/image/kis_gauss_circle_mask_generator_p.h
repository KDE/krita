/*
 *  SPDX-FileCopyrightText: 2008-2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_GAUSS_MASK_GENERATOR_P_H_
#define _KIS_GAUSS_MASK_GENERATOR_P_H_

#include "kis_antialiasing_fade_maker.h"
#include "kis_brush_mask_applicator_base.h"

struct Q_DECL_HIDDEN KisGaussCircleMaskGenerator::Private
{
    Private(bool enableAntialiasing)
        : fadeMaker(*this, enableAntialiasing)
    {
    }

    Private(const Private &rhs)
        : ycoef(rhs.ycoef),
        fade(rhs.fade),
        center(rhs.center),
        distfactor(rhs.distfactor),
        alphafactor(rhs.alphafactor),
        fadeMaker(rhs.fadeMaker, *this)
    {
    }

    qreal ycoef {0.0};
    qreal fade {0.0};
    qreal center {0.0};
    qreal distfactor {0.0};
    qreal alphafactor {0.0};
    KisAntialiasingFadeMaker1D<Private> fadeMaker;

    QScopedPointer<KisBrushMaskApplicatorBase> applicator;

    inline quint8 value(qreal dist) const;

};

#endif /* _KIS_GAUSS_MASK_GENERATOR_P_H_ */
