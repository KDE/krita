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

#include "kis_illuminant_profile.h"
#include <KoColorProfile.h>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_cblas.h>
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

    // TODO Add channels
}

KisKS3ColorSpace::~KisKS3ColorSpace()
{
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
}

void KisKS3ColorSpace::toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
}

bool KisKS3ColorSpace::operator==(const KoColorSpace& rhs) const
{
    return (rhs.id() == id()) && (*rhs.profile() == *profile());
}
