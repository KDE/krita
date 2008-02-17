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

#ifndef KIS_KSF32_COLORSPACE_H_
#define KIS_KSF32_COLORSPACE_H_

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace.h"

#include "kis_rgb_to_ks_color_conversion_transformation.h"
#include "kis_ks_to_rgb_color_conversion_transformation.h"

template< int _N_ >
class KisKSF32ColorSpace : public KisKSColorSpace< float,_N_ >
{
    typedef KisKSColorSpace< float,_N_ > parent;

    public:

        KisKSF32ColorSpace(KoColorProfile *p) : parent(p) { return; }
        ~KisKSF32ColorSpace() { return; }

        KoColorSpace *clone() const
        {
            return new KisKSF32ColorSpace<_N_>(parent::profile()->clone());
        }

};

template< int _N_ >
class KisKSF32ColorSpaceFactory : public KoColorSpaceFactory
{
    public:
        QString id() const { return KisKSF32ColorSpace<_N_>::ColorSpaceId().id(); }
        QString name() const { return KisKSF32ColorSpace<_N_>::ColorSpaceId().name(); }
        KoID colorModelId() const { return KisKSF32ColorSpace<_N_>::ColorModelId(); }
        KoID colorDepthId() const { return KisKSF32ColorSpace<_N_>::ColorDepthId(); }
        bool userVisible() const { return _N_>=9; }

        int referenceDepth() const { return sizeof(float)*8; }
        bool isIcc() const { return false; }
        bool isHdr() const { return false; }

        QList<KoColorConversionTransformationFactory*> colorConversionLinks() const
        {
            QList<KoColorConversionTransformationFactory*> list;

            // RGB to KS and vice versa
            list.append(new KisRGBToKSColorConversionTransformationFactory<float,_N_>);
            list.append(new KisKSToRGBColorConversionTransformationFactory<float,_N_>);

            // To / From F16 TODO

            return list;
        }

        KoColorConversionTransformationFactory *createICCColorConversionTransformationFactory(QString _colorModelId, QString _colorDepthId) const
        {
            Q_UNUSED(_colorModelId);
            Q_UNUSED(_colorDepthId);
            return 0;
        }

        KoColorSpace *createColorSpace(const KoColorProfile *p) const
        {
            return new KisKSF32ColorSpace<_N_>(p->clone());
        }

        bool profileIsCompatible(const KoColorProfile *profile) const
        {
            const KisIlluminantProfile *p = dynamic_cast<const KisIlluminantProfile *>(profile);
            if ((!p) || (p->wavelengths() != _N_)) {
                return false;
            }
            return true;
        }

        QString defaultProfile() const
        {
            return QString("D-65 Illuminant Profile - " + QString::number(_N_) + " wavelengths - Black [11.0,0.35] - QP");
        }
};

#endif // KIS_KSF32_COLORSPACE_H_
