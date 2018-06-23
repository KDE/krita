/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
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

    qreal ycoef;
    qreal fade;
    qreal center;
    qreal distfactor;
    qreal alphafactor;
    KisAntialiasingFadeMaker1D<Private> fadeMaker;

    QScopedPointer<KisBrushMaskApplicatorBase> applicator;

    inline quint8 value(qreal dist) const;

};

#endif /* _KIS_GAUSS_MASK_GENERATOR_P_H_ */
