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

#include "kis_ks_colorspace.h"
#include "kis_illuminant_profile.h"

#include "kis_rgb_to_ks_color_conversion_transformation_ctl.h"
#include "kis_ks_to_rgb_color_conversion_transformation_ctl.h"
#include "kis_rgb_to_ks_color_conversion_transformation.h"
#include "kis_ks_to_rgb_color_conversion_transformation.h"

#include "kis_ks_to_ks_color_conversion_transformation.h"

template< int _N_ >
class KisKSF32ColorSpace : public KisKSColorSpace< float, _N_ >
{
    typedef KisKSColorSpace< float, _N_ > parent;

public:

    KisKSF32ColorSpace(KoColorProfile *p) : parent(p) { }
    ~KisKSF32ColorSpace() { }

    KoColorSpace *clone() const {
        return new KisKSF32ColorSpace<_N_>(parent::profile()->clone());
    }

};

template< int _N_ >
class KisKSF32ColorSpaceFactory : public KisKSColorSpaceFactory<float, _N_>
{
public:

    QList<KoColorConversionTransformationFactory*> colorConversionLinks() const {
        QList<KoColorConversionTransformationFactory*> list;

        // RGB to KS and vice versa
        KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
        QString csid = KisKSF32ColorSpace<_N_>::ColorSpaceId().id();
        foreach(const KoColorProfile *p, f->profilesFor(csid)) {
            list.append(new KisCtlRGBToKSColorConversionTransformationFactory<float, _N_>(p->name()));
            list.append(new KisKSToCtlRGBColorConversionTransformationFactory<float, _N_>(p->name()));
            list.append(new KisRGBToKSColorConversionTransformationFactory<float, _N_>(p->name()));
            list.append(new KisKSToRGBColorConversionTransformationFactory<float, _N_>(p->name()));
        }
        /*
                    #ifdef HAVE_OPENEXR
                    // From F16
                    list.append(new KisKSToKSColorConversionTransformationFactory<half,float,_N_>);
                    #endif

                    // Self to self (profile change)
                    list.append(new KisKSToKSColorConversionTransformationFactory<float,float,_N_>);
        */
        return list;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new KisKSF32ColorSpace<_N_>(p->clone());
    }

};

#endif // KIS_KSF32_COLORSPACE_H_
