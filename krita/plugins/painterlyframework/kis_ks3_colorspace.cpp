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

#include "kis_ks3_colorspace.h"

#include "ks_reflectance_converter.h"
#include "kis_illuminant_profile.h"
#include <KoColorProfile.h>

#include "KoColorSpaceConstants.h"
#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

KisKS3ColorSpace::KisKS3ColorSpace(KoColorProfile *p)
: parent( "ks3colorspace",
          "KS Color Space - 3 wavelenghts",
          KoColorSpaceRegistry::instance()->rgb16("") )
{
    if (!profileIsCompatible(p))
        return;

    m_profile = dynamic_cast<KisIlluminantProfile *>(p);
    m_converter = KSReflectanceConverter(m_profile->Swhite(), m_profile->Kblack());

    m_permutation = gsl_permutation_alloc(3);
    gsl_linalg_LU_decomp(m_profile->T(), m_permutation, &m_s);

    for (quint32 i = 0; i < 2*3; i+=2) {
        addChannel(new KoChannelInfo(i18n("Absorption"),
                   i+0 * sizeof(float),
                   KoChannelInfo::COLOR,
                   KoChannelInfo::FLOAT32,
                   sizeof(float),
                   QColor(0,0,255)));

        addChannel(new KoChannelInfo(i18n("Scattering"),
                   i+1 * sizeof(float),
                   KoChannelInfo::COLOR,
                   KoChannelInfo::FLOAT32,
                   sizeof(float),
                   QColor(255,0,0)));
    }

    addChannel(new KoChannelInfo(i18n("Alpha"),
               2 * 3 * sizeof(float),
               KoChannelInfo::ALPHA,
               KoChannelInfo::FLOAT32,
               sizeof(float)));

    addCompositeOp( new KoCompositeOpOver<KisKS3ColorSpaceTrait>(this) );
    addCompositeOp( new KoCompositeOpErase<KisKS3ColorSpaceTrait>(this) );
    addCompositeOp( new KoCompositeOpMultiply<KisKS3ColorSpaceTrait>(this) );
    addCompositeOp( new KoCompositeOpDivide<KisKS3ColorSpaceTrait>(this) );
    addCompositeOp( new KoCompositeOpBurn<KisKS3ColorSpaceTrait>(this) );
}

KisKS3ColorSpace::~KisKS3ColorSpace()
{
    gsl_permutation_free(m_permutation);
}

KoColorProfile *KisKS3ColorSpace::profile()
{
    return m_profile;
}

const KoColorProfile *KisKS3ColorSpace::profile() const
{
    return const_cast<const KisIlluminantProfile *>(m_profile);
}

bool KisKS3ColorSpace::profileIsCompatible(const KoColorProfile *profile) const
{
    const KisIlluminantProfile *p = dynamic_cast<const KisIlluminantProfile *>(profile);
    if (!p)
        return false;
    if (p->wavelenghts() != 3)
        return false;
    return true;
}

void KisKS3ColorSpace::fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
    // For each pixel we do this:
    // 1 - convert raw bytes to quint16
    // 2 - find reflectances using the T matrix of the profile
    // 3 - convert reflectances to K/S

    const quint16 *srcU16 = reinterpret_cast<const quint16 *>(srcU8);
    quint8     *dst = dstU8;
    gsl_vector *src = gsl_vector_alloc(3);
    gsl_vector *ref = gsl_vector_alloc(3);

    for (quint32 pixel = 0; pixel < nPixels; pixel++) {
        gsl_vector_set(src, 0, (double)srcU16[2]);
        gsl_vector_set(src, 1, (double)srcU16[1]);
        gsl_vector_set(src, 2, (double)srcU16[0]);

        gsl_linalg_LU_solve(m_profile->T(), m_permutation, src, ref);
        for (int i = 0; i < 3; i++)
            m_converter.reflectanceToKS((float)gsl_vector_get(ref, i),
                                        KisKS3ColorSpaceTrait::K(dst, i),
                                        KisKS3ColorSpaceTrait::S(dst, i));
        KisKS3ColorSpaceTrait::setAlpha(dst, srcU16[3]>>8, 1);

        srcU16 += 4;
        dst += KisKS3ColorSpaceTrait::pixelSize;
    }

    gsl_vector_free(ref);
    gsl_vector_free(src);
}

void KisKS3ColorSpace::toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
}

bool KisKS3ColorSpace::operator==(const KoColorSpace& rhs) const
{
    return (rhs.id() == id()) && (*rhs.profile() == *profile());
}
