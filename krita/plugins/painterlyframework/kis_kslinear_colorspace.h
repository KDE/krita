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

#define KS3LINEARID KoID("KS3LINEAR", i18n("3-pairs Absorption-Scattering Linear"))

class KisKSLinearColorSpace : public KisKSColorSpace<3>
{
    typedef KisKSColorSpace<3> parent;

    public:

        KisKSLinearColorSpace(KoColorProfile *p) :
        parent(p, colorSpaceId(), i18n("3-pairs Absorption-Scattering Linear (32 Bits Float)")) {}

        ~KisKSLinearColorSpace() {}

        KoID colorModelId() const
        {
            return KS3LINEARID;
        }

        KoColorSpace* clone() const
        {
            return new KisKSLinearColorSpace(const_cast<KoColorProfile*>(profile()));
        }

        static QString colorSpaceId()
        {
            return "KS9QPF32";
        }

};

class KisKSLinearColorSpaceFactory : public KoColorSpaceFactory
{
    public:
        QString id() const { return KisKSLinearColorSpace::colorSpaceId(); }
        QString name() const { return i18n("3-pairs Absorption-Scattering Linear (32 Bits Float)"); }
        KoID colorModelId() const { return KS3LINEARID; }
        KoID colorDepthId() const { return Float32BitsColorDepthID; }
        bool userVisible() const { return true; }

        int referenceDepth() const { return 32; }
        bool isIcc() const { return false; }
        bool isHdr() const { return false; }

        QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;

        KoColorConversionTransformationFactory *createICCColorConversionTransformationFactory(QString _colorModelId, QString _colorDepthId) const
        {
            Q_UNUSED(_colorModelId);
            Q_UNUSED(_colorDepthId);
            return 0;
        }

        KoColorSpace *createColorSpace(const KoColorProfile *p) const {
            return new KisKSLinearColorSpace(const_cast<KoColorProfile*>(p));
        }

        bool profileIsCompatible(const KoColorProfile *profile) const
        {
            return KisKSLinearColorSpace(0).profileIsCompatible(profile);
        }

        QString defaultProfile() const { return ""; } // TODO
};

#endif // KIS_KSLINEAR_COLORSPACE_H_
