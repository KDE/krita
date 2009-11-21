/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef KIS_KS_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_
#define KIS_KS_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace.h"

#include <KoColorConversionTransformation.h>
#include <KoColorConversionTransformationFactory.h>

#include "kis_debug.h"

template< typename _TYPE_, int _N_ >
class KisKSToRGBColorConversionTransformation : public KoColorConversionTransformation
{

    typedef KoColorConversionTransformation parent;
    typedef KisKSColorSpaceTrait<_TYPE_, _N_> CSTrait;

public:

    KisKSToRGBColorConversionTransformation(const KoColorSpace *srcCs, const KoColorSpace *dstCs)
            : parent(srcCs, dstCs), m_rgbvec(0), m_ksvec(0), m_srcProfile(0) {
        m_srcProfile = static_cast<const KisIlluminantProfile*>(parent::srcColorSpace()->profile());
        m_rgbvec = new double[3];
        m_ksvec  = new double[2*_N_];
    }

    ~KisKSToRGBColorConversionTransformation() {
        delete [] m_rgbvec;
        delete [] m_ksvec;
    }

    void transform(const quint8 *src, quint8 *dst8, int nPixels) const {
        quint16 *dst = reinterpret_cast<quint16*>(dst8);

        for (; nPixels > 0; nPixels--) {

            for (int i = 0; i < _N_; i++) {
                m_ksvec[2*i+0] = (double)CSTrait::K(src, i);
                m_ksvec[2*i+1] = (double)CSTrait::S(src, i);
            }

            m_srcProfile->toRgb(m_ksvec, m_rgbvec);

            dst[0] = KoColorSpaceMaths<_TYPE_, quint16 >::scaleToA(m_rgbvec[2]);
            dst[1] = KoColorSpaceMaths<_TYPE_, quint16 >::scaleToA(m_rgbvec[1]);
            dst[2] = KoColorSpaceMaths<_TYPE_, quint16 >::scaleToA(m_rgbvec[0]);
            dst[3] = KoColorSpaceMaths<_TYPE_, quint16 >::scaleToA(CSTrait::nativealpha(src));

            src += CSTrait::pixelSize;
            dst += 4;

        }

    }

private:
    double *m_rgbvec;
    double *m_ksvec;

    const KisIlluminantProfile *m_srcProfile;

};

template< typename _TYPE_, int _N_ >
class KisKSToRGBColorConversionTransformationFactory : public KoColorConversionTransformationFactory
{

public:
    KisKSToRGBColorConversionTransformationFactory(QString srcProfile)
            : KoColorConversionTransformationFactory(QString("KS%1").arg(_N_),
                    KisKSColorSpace<_TYPE_, _N_>::ColorDepthId().id(), srcProfile,
                    RGBAColorModelID.id(),
                    Integer16BitsColorDepthID.id(), "sRGB built-in - (lcms internal)") { }

    KoColorConversionTransformation *createColorTransformation(const KoColorSpace* srcColorSpace,
            const KoColorSpace* dstColorSpace,
            KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const {
        Q_UNUSED(renderingIntent);

        Q_ASSERT(canBeSource(srcColorSpace));
        Q_ASSERT(canBeDestination(dstColorSpace));

        return new KisKSToRGBColorConversionTransformation<_TYPE_, _N_>(srcColorSpace, dstColorSpace);
    }

    bool conserveColorInformation() const {
        return true;
    }
    bool conserveDynamicRange() const {
        return false;
    }

};

#endif // KIS_KS_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_
