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

#include <cmath>

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

extern "C" {
    #include "cqp/gsl_cqp.h"
}

class KisIlluminantProfile;
class KoColorProfile;

template<int _N_>
class KisKSColorSpace : public KoIncompleteColorSpace< KisKSColorSpaceTrait<_N_> >
{
    typedef KoIncompleteColorSpace< KisKSColorSpaceTrait<_N_> > parent;

    public:

        KisKSColorSpace(KoColorProfile *p, const QString &id = "kscolorspace", const QString &name = "KS Color Space");
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
        /**
         * Overload this in subclasses if you want to accelerate the conversion
         * (see KisKS3ColorSpace for instance). Use it with m_rgbvec and m_refvec.
         */
        virtual void RGBToReflectance() const;

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
bool KisKSColorSpace<_N_>::operator==(const KoColorSpace& rhs) const
{
    return (rhs.id() == parent::id()) && (*rhs.profile() == *profile());
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

        KisKSColorSpaceTrait<_N_>::setAlpha(dst, KoColorSpaceMaths<quint16,quint8>::scaleToA(srcU16[3]), 1);

        srcU16 += 4;
        dst += KisKSColorSpaceTrait<_N_>::pixelSize;
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
        dstU16[3] = KoColorSpaceMaths<float,quint16>::scaleToA(KisKSColorSpaceTrait<_N_>::alpha(src));

        src += KisKSColorSpaceTrait<_N_>::pixelSize;
        dstU16 += 4;
    }
}

template<int _N_>
void KisKSColorSpace<_N_>::RGBToReflectance() const
{
    int n = m_refvec->size;
    int me = m_rgbvec->size; // == 3
    int mi = 2*n; // each reflectance is bounded between 0 and 1, two inequalities.

    //// ALLOCATION
    gsl_cqp_data *data = new gsl_cqp_data;
    data->Q = gsl_matrix_calloc(n,n);
    data->q = gsl_vector_calloc(n); // This will remain zero
    data->A = gsl_matrix_calloc(me,n);
    data->C = gsl_matrix_calloc(mi,n);
    data->b = gsl_vector_calloc(me);
    data->d = gsl_vector_calloc(mi);

    //// INITIALIZATION
    // The Q matrix represents this function:
    // z = ( R_P[0] - R_P[1] )^2 + ( R_P[1] - R_P[2] )^2 + ... + ( R_P[n-2] - R_P[n-1] )^2
    for (int i = 1; i < n; i++) {
        int prev = (int)gsl_vector_get(m_profile->P(), i-1);
        int curr = (int)gsl_vector_get(m_profile->P(), i);
//         int prev = i-1, curr = i;
        gsl_matrix_set(data->Q, prev, prev, gsl_matrix_get(data->Q,prev,prev)+1.0);
//         gsl_matrix_set(data->Q, prev, prev,  2.0);
        gsl_matrix_set(data->Q, curr, curr,  1.0);
        gsl_matrix_set(data->Q, prev, curr, -1.0);
        gsl_matrix_set(data->Q, curr, prev, -1.0);
    }

    // The A matrix and b vector represent the equalities that the variables must
    // consider. In our case, they're exactly the transformation matrix and the rgb vector
    gsl_matrix_memcpy(data->A, m_profile->T());
    gsl_vector_memcpy(data->b, m_rgbvec);

    // The C matrix and d vector represent the inequalities that the variables must
    // consider. In our case, they're: x_i >= 0, -x_i >= -1 (you can only define >= inequalities)
    for (int i = 0; i < n; i++) {
        int j = 2*i;
        gsl_matrix_set(data->C, j+0, i,  1.0);
        gsl_matrix_set(data->C, j+1, i, -1.0);

        gsl_vector_set(data->d, j+0,  0.0);
        gsl_vector_set(data->d, j+1, -1.0);
    }

    //// SOLVER INITIALIZATION
    const size_t max_iter = 1000;
    size_t iter=1;
    int status;
    const gsl_cqpminimizer_type *T;
    gsl_cqpminimizer *s;

    T = gsl_cqpminimizer_mg_pdip;
    s = gsl_cqpminimizer_alloc(T, n, me, mi);
    status = gsl_cqpminimizer_set(s, data);

    do {
        status = gsl_cqpminimizer_iterate(s);
        status = gsl_cqpminimizer_test_convergence(s, 1e-10, 1e-10);

        if(status != GSL_SUCCESS)
            iter++;
    } while(status == GSL_CONTINUE && iter<=max_iter);

    for (int i = 0; i < n; i++) {
        double curr = gsl_vector_get(gsl_cqpminimizer_x(s), i);

        if (fabs(curr - 0.0) < 1e-6)
            gsl_vector_set(m_refvec, i, 0.0);
        else if (fabs(curr - 1.0) < 1e-6)
            gsl_vector_set(m_refvec, i, 1.0);
        else
            gsl_vector_set(m_refvec, i, curr);
    }

    gsl_cqpminimizer_free(s);

    gsl_vector_free(data->d);
    gsl_vector_free(data->b);
    gsl_matrix_free(data->C);
    gsl_matrix_free(data->A);
    gsl_vector_free(data->q);
    gsl_matrix_free(data->Q);
    delete data;
}

typedef KisKSColorSpace<9>  KisKS9ColorSpace;
typedef KisKSColorSpace<12> KisKS12ColorSpace;

#endif // KIS_KS_COLORSPACE_H_
