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

#ifndef KIS_KSLINEAR_COLORSPACE_H_
#define KIS_KSLINEAR_COLORSPACE_H_

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace_traits.h"
#include "kis_ks_colorspace.h"

class KisKSLinearColorSpace : public KisKSColorSpace<3>
{
    typedef KisKSColorSpace<3> parent;

    public:

        KisKSLinearColorSpace(KoColorProfile *p) :
        parent(p, "KS3LINEARF32", i18n("3-pairs Absorption-Scattering Linear (32 Bits Float)")) {}

        ~KisKSLinearColorSpace() {}

        KoID colorModelId() const
        {
            return KoID("KS3LINEAR", i18n("3-pairs Absorption-Scattering Linear"));
        }

        KoColorSpace* clone() const
        {
            return new KisKSLinearColorSpace(const_cast<KoColorProfile*>(profile()));
        }
};

#endif // KIS_KSLINEAR_COLORSPACE_H_
