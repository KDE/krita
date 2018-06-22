/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2011 Geoffry Song <goffrie@gmail.com>
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

    qreal xfade, yfade;
    qreal halfWidth, halfHeight;
    qreal alphafactor;

    KisAntialiasingFadeMaker2D <Private> fadeMaker;

    QScopedPointer<KisBrushMaskApplicatorBase> applicator;

    inline quint8 value(qreal x, qreal y) const;

    #if defined HAVE_VC
    // vectorized erf function, precision 1e-5
    Vc::float_v vErf(Vc::float_v x) {
        Vc::float_v xa = abs(x);
        Vc::float_m precisionLimit(xa >= 9.3f); // wrong result for any number beyond this
        xa(precisionLimit) = 0;
        Vc::float_v sign(Vc::One);
        Vc::float_m invertMask = x < 0.f;
        sign(invertMask) = -1.f;

        // CONSTANTS
        float a1 =  0.254829592;
        float a2 = -0.284496736;
        float a3 =  1.421413741;
        float a4 = -1.453152027;
        float a5 =  1.061405429;
        float p  =  0.3275911;

        Vc::float_v t = 1.0f / (1.0f + p * xa);
        Vc::float_v y = 1.0f - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-xa * xa);
        y(precisionLimit) = 1.0f;
        return sign * y;
    }
    #endif /* defined HAVE_VC */
};

#endif // KIS_GAUSS_RECT_MASK_GENERATOR_P_H
