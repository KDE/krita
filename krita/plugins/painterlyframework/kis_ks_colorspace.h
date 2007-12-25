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

#ifndef KIS_KS_COLORSPACE_H_
#define KIS_KS_COLORSPACE_H_

#include <KoIncompleteColorSpace.h>

#include "kis_ks_colorspace_traits.h"
#include "channel_converter.h"

#include "kis_illuminant_profile.h"
#include <KoColorSpaceMaths.h>

#include "KoColorSpaceConstants.h"
#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

class KisIlluminantProfile;
class KoColorProfile;

template<int N>
class KisKSColorSpace : public KoIncompleteColorSpace< KisKSColorSpaceTrait<N> >
{
    typedef KoIncompleteColorSpace< KisKSColorSpaceTrait<N> > parent;

    public:

        KisKSColorSpace(KoColorProfile *p, const QString &id, const QString &name);
        virtual ~KisKSColorSpace();

    public:

        virtual bool operator==(const KoColorSpace& rhs) const;

        virtual KoColorProfile *profile();
        virtual const KoColorProfile *profile() const;
        virtual bool profileIsCompatible(const KoColorProfile *profile) const;

        virtual KoColorSpace* clone() const { return 0; }
        virtual KoID colorModelId() const { return KoID(); }
        virtual KoID colorDepthId() const { return KoID(); }
        virtual bool willDegrade(ColorSpaceIndependence) const { return true; }
        virtual void colorToXML(const quint8*, QDomDocument&, QDomElement&) const {}
        virtual void colorFromXML(quint8*, const QDomElement&) const {}

        virtual void fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;
        virtual void toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;

    protected:
        // Overload this in subclasses if you want to accelerate the conversion
        // (see KisKS3ColorSpace for instance)
        virtual void RGBToReflectance(gsl_vector *rgb, gsl_vector *ref) const;

    protected:

        KisIlluminantProfile *m_profile;
        ChannelConverter m_converter;

        gsl_vector *m_rgbvec;
        gsl_vector *m_refvec;

};

////////////////////////////////////////////
//            IMPLEMENTATION              //
////////////////////////////////////////////

#define NORM 65535.0f

template<int N>
KisKSColorSpace<N>::KisKSColorSpace(KoColorProfile *p, const QString &id, const QString &name)
: parent(id, name, KoColorSpaceRegistry::instance()->rgb16("")), m_rgbvec(0), m_refvec(0)
{
    if (!profileIsCompatible(p))
        return;

    m_profile = dynamic_cast<KisIlluminantProfile *>(p);
    m_converter = ChannelConverter(m_profile->Swhite(), m_profile->Kblack());

    m_rgbvec = gsl_vector_alloc(N);
    m_refvec = gsl_vector_alloc(N);

    for (quint32 i = 0; i < 2*N; i+=2) {
        parent::addChannel(new KoChannelInfo(i18n("Absorption"),
                           i+0 * sizeof(float),
                           KoChannelInfo::COLOR,
                           KoChannelInfo::FLOAT32,
                           sizeof(float),
                           QColor(0,0,255)));

        parent::addChannel(new KoChannelInfo(i18n("Scattering"),
                           i+1 * sizeof(float),
                           KoChannelInfo::COLOR,
                           KoChannelInfo::FLOAT32,
                           sizeof(float),
                           QColor(255,0,0)));
    }

    parent::addChannel(new KoChannelInfo(i18n("Alpha"),
                       2 * N * sizeof(float),
                       KoChannelInfo::ALPHA,
                       KoChannelInfo::FLOAT32,
                       sizeof(float)));

    addCompositeOp( new KoCompositeOpOver< KisKSColorSpaceTrait<N> >(this) );
    addCompositeOp( new KoCompositeOpErase< KisKSColorSpaceTrait<N> >(this) );
    addCompositeOp( new KoCompositeOpMultiply< KisKSColorSpaceTrait<N> >(this) );
    addCompositeOp( new KoCompositeOpDivide< KisKSColorSpaceTrait<N> >(this) );
    addCompositeOp( new KoCompositeOpBurn< KisKSColorSpaceTrait<N> >(this) );
}

template<int N>
KisKSColorSpace<N>::~KisKSColorSpace()
{
    if (m_rgbvec)
        gsl_vector_free(m_rgbvec);
    if (m_refvec)
        gsl_vector_free(m_refvec);
}

template<int N>
bool KisKSColorSpace<N>::operator==(const KoColorSpace& rhs) const
{
    return (rhs.id() == parent::id()) && (*rhs.profile() == *profile());
}

template<int N>
KoColorProfile *KisKSColorSpace<N>::profile()
{
    return m_profile;
}

template<int N>
const KoColorProfile *KisKSColorSpace<N>::profile() const
{
    return const_cast<const KisIlluminantProfile *>(m_profile);
}

template<int N>
bool KisKSColorSpace<N>::profileIsCompatible(const KoColorProfile *profile) const
{
    const KisIlluminantProfile *p = dynamic_cast<const KisIlluminantProfile *>(profile);
    if (!p)
        return false;
    if (p->wavelenghts() != N)
        return false;
    return true;
}

template<int N>
void KisKSColorSpace<N>::fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
    // For each pixel we do this:
    // 1 - convert raw bytes to quint16
    // 2 - find reflectances using the T matrix of the profile
    // 3 - convert reflectances to K/S

    const quint16 *srcU16 = reinterpret_cast<const quint16 *>(srcU8);
    quint8 *dst = dstU8;
    float c;

    for ( ; nPixels > 0; nPixels-- ) {
        for (int i = 0; i < 3; i++) {
            m_converter.sRGBToRGB((float)srcU16[2-i]/NORM, c);
            gsl_vector_set(m_rgbvec, i, (double)c);
        }

        RGBToReflectance(m_rgbvec, m_refvec);

        for (int i = 0; i < 3; i++)
            m_converter.reflectanceToKS((float)gsl_vector_get(m_refvec, i),
                                        KisKSColorSpaceTrait<N>::K(dst, i),
                                        KisKSColorSpaceTrait<N>::S(dst, i));

        KisKSColorSpaceTrait<N>::setAlpha(dst, KoColorSpaceMaths<quint16,quint8>::scaleToA(srcU16[3]), 1);

        srcU16 += 4;
        dst += KisKSColorSpaceTrait<N>::pixelSize;
    }
}

template<int N>
void KisKSColorSpace<N>::toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
    quint16 *dstU16 = reinterpret_cast<quint16 *>(dstU8);
    const quint8 *src = srcU8;
    float c;

    for ( ; nPixels > 0; nPixels-- ) {
        for (int i = 0; i < 3; i++) {
            m_converter.KSToReflectance(KisKSColorSpaceTrait<N>::K(src, i),
                                        KisKSColorSpaceTrait<N>::S(src, i), c);
            gsl_vector_set(m_refvec, i, (double)c);
        }

        gsl_blas_dgemv(CblasNoTrans, 1.0, m_profile->T(), m_refvec, 0.0, m_rgbvec);

        for (int i = 0; i < 3; i++) {
            m_converter.RGBTosRGB((float)gsl_vector_get(m_rgbvec, i), c);
            dstU16[2-i] = (quint16)(c * NORM);
        }
        dstU16[3] = KoColorSpaceMaths<float,quint16>::scaleToA(KisKSColorSpaceTrait<N>::alpha(src));

        src += KisKSColorSpaceTrait<N>::pixelSize;
        dstU16 += 4;
    }
}

template<int N>
void KisKSColorSpace<N>::RGBToReflectance(gsl_vector *rgb, gsl_vector *ref) const
{
    Q_UNUSED(rgb);
    Q_UNUSED(ref);
}

#undef NORM

typedef KisKSColorSpace<9>  KisKS9ColorSpace;
typedef KisKSColorSpace<12> KisKS12ColorSpace;

#endif // KIS_KS_COLORSPACE_H_
