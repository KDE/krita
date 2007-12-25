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
#include "channel_converter.h"
#include <KoIncompleteColorSpace.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

class KisIlluminantProfile;
class KoColorProfile;

class KisKS3ColorSpace : public KoIncompleteColorSpace<KisKS3ColorSpaceTrait>
{
    typedef KoIncompleteColorSpace<KisKS3ColorSpaceTrait> parent;

    public:

        KisKS3ColorSpace(KoColorProfile *p);
        ~KisKS3ColorSpace();

    public:

        bool operator==(const KoColorSpace& rhs) const;

        KoColorProfile *profile();
        const KoColorProfile *profile() const;
        bool profileIsCompatible(const KoColorProfile *profile) const;

        KoColorSpace* clone() const { return 0; }
        KoID colorModelId() const { return KoID(); }
        KoID colorDepthId() const { return KoID(); }
        bool willDegrade(ColorSpaceIndependence) const { return true; }
        void colorToXML(const quint8*, QDomDocument&, QDomElement&) const {}
        void colorFromXML(quint8*, const QDomElement&) const {}

        void fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;
        void toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;

    private:

        KisIlluminantProfile *m_profile;
        ChannelConverter m_converter;

        gsl_matrix *m_inverse;
        gsl_vector *m_rgbvec;
        gsl_vector *m_refvec;

};

#endif // KIS_KS3_COLORSPACE_H_
