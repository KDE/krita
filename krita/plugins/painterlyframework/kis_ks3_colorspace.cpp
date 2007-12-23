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

KisKS3ColorSpace::KisKS3ColorSpace(KoColorProfile *p)
: parent( "KS3CS" + p->name(),
          "KS 3 Color Space with profile " + p->name(),
          KoColorSpaceRegistry::instance()->rgb16("") )
{
    m_profile = dynamic_cast<KisIlluminantProfile *>(p);

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
    if (!dynamic_cast<const KisIlluminantProfile *>(profile))
        return false;
    return true;
}

void KisKS3ColorSpace::fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
}

void KisKS3ColorSpace::toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const
{
}
