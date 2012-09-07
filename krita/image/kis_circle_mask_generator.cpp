/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
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

#include <cmath>
#include <Vc/Vc>
#include <Vc/IO>

#include <QDomDocument>

#include "kis_fast_math.h"
#include "kis_circle_mask_generator.h"
#include "kis_base_mask_generator.h"

    inline Vc::float_v normeSIMD(Vc::float_v a, Vc::float_v b) {
        return a*a + b*b;
    }

struct KisCircleMaskGenerator::Private {
    double xcoef, ycoef;
    double xfadecoef, yfadecoef;
    double transformedFadeX, transformedFadeY;
};

KisCircleMaskGenerator::KisCircleMaskGenerator(qreal diameter, qreal ratio, qreal fh, qreal fv, int spikes)
        : KisMaskGenerator(diameter, ratio, fh, fv, spikes, CIRCLE, DefaultId), d(new Private)
{
    d->xcoef = 2.0 / width();
    d->ycoef = 2.0 / (KisMaskGenerator::d->ratio * width());
    d->xfadecoef = (KisMaskGenerator::d->fh == 0) ? 1 : (1.0 / (KisMaskGenerator::d->fh * width()));
    d->yfadecoef = (KisMaskGenerator::d->fv == 0) ? 1 : (1.0 / (KisMaskGenerator::d->fv * KisMaskGenerator::d->ratio * width()));
    d->transformedFadeX = d->xfadecoef * softness();
    d->transformedFadeY = d->yfadecoef * softness();
}

KisCircleMaskGenerator::~KisCircleMaskGenerator()
{
    delete d;
}

bool KisCircleMaskGenerator::shouldSupersample() const
{
    return width() < 10 || KisMaskGenerator::d->ratio * width() < 10;
}

quint8 KisCircleMaskGenerator::valueAt(qreal x, qreal y) const
{
    if (KisMaskGenerator::d->empty) return 255;
    double xr = (x /*- m_xcenter*/);
    double yr = fabs(y /*- m_ycenter*/);

    if (KisMaskGenerator::d->spikes > 2) {
        double angle = (KisFastMath::atan2(yr, xr));

        while (angle > KisMaskGenerator::d->cachedSpikesAngle ){
            double sx = xr;
            double sy = yr;

            xr = KisMaskGenerator::d->cs * sx - KisMaskGenerator::d->ss * sy;
            yr = KisMaskGenerator::d->ss * sx + KisMaskGenerator::d->cs * sy;

            angle -= 2 * KisMaskGenerator::d->cachedSpikesAngle;
        }
    }

    double n = norme(xr * d->xcoef, yr * d->ycoef);

    if (n > 1) {
        return 255;
    } else {
        double normeFade = norme(xr * d->transformedFadeX, yr * d->transformedFadeY);
        if (normeFade > 1) {
            // xle stands for x-coordinate limit exterior
            // yle stands for y-coordinate limit exterior
            // we are computing the coordinate on the external ellipse in order to compute
            // the fade value
            // xle = xr / sqrt(norme(xr * d->xcoef, yr * d->ycoef))
            // yle = yr / sqrt(norme(xr * d->xcoef, yr * d->ycoef))

            // On the internal limit of the fade area, normeFade is equal to 1

            // normeFadeLimitE = norme(xle * transformedFadeX, yle * transformedFadeY)
            // return (uchar)(255 *(normeFade - 1) / (normeFadeLimitE - 1));
            return (uchar)(255 * n * (normeFade - 1) / (normeFade - n));
            // if n == 0, the conversion of NaN to uchar will correctly result in zero
        } else {
            n = 1 - n;
            if( width() < 2 || height() < 2 || n > d->xcoef * 0.5 || n > d->ycoef * 0.5)
            {
              return 0;
            } else {
              return 255 *  ( 1 - 4 * n * n  / (d->xcoef * d->ycoef) );
            }
        }
    }
}

void KisCircleMaskGenerator::processRowFast(float* buffer, int width, float y, float cosa, float sina,
                                            float centerX, float centerY, float invScaleX, float invScaleY)
{
    float y_ = (y - centerY) * invScaleY;
    float sinay_ = sina * y_;
    float cosay_ = cosa * y_;

    float *initValues = Vc::malloc<float, Vc::AlignOnVector>(Vc::float_v::Size);
    for(int i = 0; i < Vc::float_v::Size; i++) {
        initValues[i] = (float)i;
    }

    float* bufferPointer = buffer;

    Vc::float_v currentIndices(initValues);

    Vc::float_v increment((float)Vc::float_v::Size);
    Vc::float_v vCenterX(centerX);
    Vc::float_v vInvScaleX(invScaleX);

    Vc::float_v vCosa(cosa);
    Vc::float_v vSina(sina);
    Vc::float_v vCosaY_(cosay_);
    Vc::float_v vSinaY_(sinay_);

    Vc::float_v vXCoeff(d->xcoef);
    Vc::float_v vYCoeff(d->ycoef);

    Vc::float_v vTransformedFadeX(d->transformedFadeX);
    Vc::float_v vTransformedFadeY(d->transformedFadeY);

    Vc::float_v vOne(1.0f);

    for (int i=0; i < width; i+= Vc::float_v::Size){

        Vc::float_v x_ = (currentIndices - vCenterX) * vInvScaleX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = x_ * vSina + vCosaY_;

        Vc::float_v n = normeSIMD(xr * vXCoeff, yr * vYCoeff);

        Vc::float_v vNormFade = normeSIMD(xr * vTransformedFadeX, yr * vTransformedFadeY);

        //255 * n * (normeFade - 1) / (normeFade - n)
        Vc::float_v vFade = n * (vNormFade - vOne) / (vNormFade - n);
        // Mask out the inner circe of the mask
        Vc::float_m mask = vNormFade < vOne;
        vFade.setZero(mask);
        vFade = Vc::min(vFade, vOne);

        vFade.store(bufferPointer);
        currentIndices = currentIndices + increment;

        bufferPointer += Vc::float_v::Size;
    }
}


void KisCircleMaskGenerator::toXML(QDomDocument& d, QDomElement& e) const
{
    KisMaskGenerator::toXML(d, e);
    e.setAttribute("type", "circle");
}

void KisCircleMaskGenerator::setSoftness(qreal softness)
{
    if (softness == 0){
        KisMaskGenerator::setSoftness(1.0);
    }else{
        KisMaskGenerator::setSoftness(1.0 / softness);
    }
    d->transformedFadeX = d->xfadecoef * this->softness();
    d->transformedFadeY = d->yfadecoef * this->softness();
}
