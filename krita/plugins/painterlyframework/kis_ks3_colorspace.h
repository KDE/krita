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
#include "kis_ks_colorspace.h"
#include "channel_converter.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

class KisIlluminantProfile;
class KoColorProfile;

typedef KisKSColorSpaceTrait<3> KisKS3ColorSpaceTrait;

class KisKS3ColorSpace : public KisKSColorSpace<3>
{
    typedef KisKSColorSpace<3> parent;

    public:

        KisKS3ColorSpace(KoColorProfile *p);
        ~KisKS3ColorSpace();

    protected:
        void RGBToReflectance() const;

    private:
        gsl_matrix *m_inverse;

};

#endif // KIS_KS3_COLORSPACE_H_
