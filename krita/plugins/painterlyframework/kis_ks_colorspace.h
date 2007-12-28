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

#include "channel_converter.h"
#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace_traits.h"

#include <KoColorModelStandardIds.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceMaths.h>
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

template<int _N_>
class KisKSColorSpace : public KoIncompleteColorSpace< KisKSColorSpaceTrait<_N_> >
{
    typedef KoIncompleteColorSpace< KisKSColorSpaceTrait<_N_> > parent;

    public:

        KisKSColorSpace(KoColorProfile *p, const QString &id, const QString &name);
        virtual ~KisKSColorSpace();

    public:

        KoColorProfile *profile();
        const KoColorProfile *profile() const;
        bool profileIsCompatible(const KoColorProfile *profile) const;
        KoID colorDepthId() const { return Float32BitsColorDepthID; }
        void colorToXML(const quint8*, QDomDocument&, QDomElement&) const {} // TODO
        void colorFromXML(quint8*, const QDomElement&) const {} // TODO

        void fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;
        void toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;

        virtual KoID colorModelId() const = 0;
        virtual KoColorSpace* clone() const { return 0; } // TODO pure virtual
        virtual bool willDegrade(ColorSpaceIndependence) const { return false; } // TODO pure virtual

    protected:
        /**
         * Overload this in subclasses if you want to accelerate the conversion
         * (see KisKS9ColorSpace for instance). Use it with m_rgbvec and m_refvec.
         */
        virtual void RGBToReflectance() const = 0;

    protected:

        KisIlluminantProfile *m_profile;
        ChannelConverter m_converter;

        gsl_vector *m_rgbvec;
        gsl_vector *m_refvec;

};

////////////////////////////////////////////
//            IMPLEMENTATION              //
////////////////////////////////////////////

template<int _N_>
KisKSColorSpace<_N_>::KisKSColorSpace(KoColorProfile *p, const QString &id, const QString &name)
: parent(id, name, KoColorSpaceRegistry::instance()->rgb16("")), m_rgbvec(0), m_refvec(0)
{
    if (!profileIsCompatible(p))
        return;

    m_profile = dynamic_cast<KisIlluminantProfile *>(p);
    m_converter = ChannelConverter(m_profile->Kblack(), m_profile->Sblack());

    m_rgbvec = gsl_vector_alloc(3);
    m_refvec = gsl_vector_alloc(_N_);

    for (quint32 i = 0; i < 2*_N_; i+=2) {
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
                       2 * _N_ * sizeof(float),
                       KoChannelInfo::ALPHA,
                       KoChannelInfo::FLOAT32,
                       sizeof(float)));

    addCompositeOp( new KoCompositeOpOver< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpErase< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpMultiply< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpDivide< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpBurn< KisKSColorSpaceTrait<_N_> >(this) );
}

template<int _N_>
KisKSColorSpace<_N_>::~KisKSColorSpace()
{
    if (m_rgbvec)
        gsl_vector_free(m_rgbvec);
    if (m_refvec)
        gsl_vector_free(m_refvec);
}

template<int _N_>
KoColorProfile *KisKSColorSpace<_N_>::profile()
{
    return m_profile;
}

template<int _N_>
const KoColorProfile *KisKSColorSpace<_N_>::profile() const
{
    return const_cast<const KisIlluminantProfile *>(m_profile);
}

template<int _N_>
bool KisKSColorSpace<_N_>::profileIsCompatible(const KoColorProfile *profile) const
{
    const KisIlluminantProfile *p = dynamic_cast<const KisIlluminantProfile *>(profile);
    if (!p)
        return false;
    if (p->wavelenghts() != _N_)
        return false;
    return true;
}

template<int _N_>
void KisKSColorSpace<_N_>::fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
    // For each pixel we do this:
    // 1 - convert raw bytes to quint16
    // 2 - find reflectances solving a Quadratic Program over the transformation matrix of the profile.
    // 3 - convert reflectances to K/S

    const quint16 *srcU16 = reinterpret_cast<const quint16 *>(srcU8);
    quint8 *dst = dstU8;
    float c;

    for ( ; nPixels > 0; nPixels-- ) {
        // We need linear RGB values.
        for (int i = 0; i < 3; i++) {
            m_converter.sRGBToRGB(KoColorSpaceMaths<quint16,float>::scaleToA(srcU16[2-i]), c);
            gsl_vector_set(m_rgbvec, i, (double)c);
        }

        RGBToReflectance();

        for (int i = 0; i < _N_; i++)
            m_converter.reflectanceToKS((float)gsl_vector_get(m_refvec, i),
                                         KisKSColorSpaceTrait<_N_>::K(dst, i),
                                         KisKSColorSpaceTrait<_N_>::S(dst, i));

        parent::setAlpha(dst, KoColorSpaceMaths<quint16,quint8>::scaleToA(srcU16[3]), 1);

        srcU16 += 4;
        dst += parent::pixelSize();
    }
}

template<int _N_>
void KisKSColorSpace<_N_>::toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
    quint16 *dstU16 = reinterpret_cast<quint16 *>(dstU8);
    const quint8 *src = srcU8;
    float c;

    for ( ; nPixels > 0; nPixels-- ) {
        for (int i = 0; i < _N_; i++) {
            m_converter.KSToReflectance(KisKSColorSpaceTrait<_N_>::K(src, i),
                                        KisKSColorSpaceTrait<_N_>::S(src, i), c);
            gsl_vector_set(m_refvec, i, (double)c);
        }

        gsl_blas_dgemv(CblasNoTrans, 1.0, m_profile->T(), m_refvec, 0.0, m_rgbvec);

        for (int i = 0; i < 3; i++) {
            m_converter.RGBTosRGB((float)gsl_vector_get(m_rgbvec, i), c);
            dstU16[2-i] = KoColorSpaceMaths<float,quint16>::scaleToA(c);
        }
        dstU16[3] = KoColorSpaceMaths<float,quint16>::scaleToA(parent::alpha(src));

        src += parent::pixelSize();
        dstU16 += 4;
    }
}

#endif // KIS_KS_COLORSPACE_H_
