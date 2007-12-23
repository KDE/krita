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

#ifndef KIS_KS3_COLORSPACE_H_
#define KIS_KS3_COLORSPACE_H_

#include "kis_ks_colorspace_traits.h"
#include <KoIncompleteColorSpace.h>

class KisIlluminantProfile;
class KoColorProfile;

class KisKS3ColorSpace : public KoIncompleteColorSpace<KisKS3ColorSpaceTrait>
{
    typedef KoIncompleteColorSpace<KisKS3ColorSpaceTrait> parent;

    public:

        KisKS3ColorSpace(KoColorProfile *p);
        ~KisKS3ColorSpace();

    public:

        KoColorProfile *profile();
        const KoColorProfile *profile() const;
        bool profileIsCompatible(const KoColorProfile *profile) const;

        void fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;
        void toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;

    private:

        KisIlluminantProfile *m_profile;

};

#endif // KIS_KS3_COLORSPACE_H_
