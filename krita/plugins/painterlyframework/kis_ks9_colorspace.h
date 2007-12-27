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

#ifndef KIS_KS9_COLORSPACE_H_
#define KIS_KS9_COLORSPACE_H_

extern "C" {
    #include "cqp/gsl_cqp.h"
}

#include "kis_ks_colorspace_traits.h"
#include "kis_ks_colorspace.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

class KisIlluminantProfile;
class KoColorProfile;

typedef KisKSColorSpaceTrait<9> KisKS9ColorSpaceTrait;

class KisKS9ColorSpace : public KisKSColorSpace<9>
{
    typedef KisKSColorSpace<9> parent;

    public:

        KisKS9ColorSpace(KoColorProfile *p);
        ~KisKS9ColorSpace();

    protected:
        void RGBToReflectance() const;

    private:
        gsl_cqp_data *m_data;
        gsl_cqpminimizer *m_s;
        const gsl_cqpminimizer_type *m_T;

};

#endif // KIS_KS9_COLORSPACE_H_
