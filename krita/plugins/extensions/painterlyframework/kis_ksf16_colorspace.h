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

#ifndef KIS_KSF16_COLORSPACE_H_
#define KIS_KSF16_COLORSPACE_H_

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace.h"

#include <config-openctl.h>
#ifdef HAVE_OPENCTL
#include "kis_rgb_to_ks_color_conversion_transformation_ctl.h"
#include "kis_ks_to_rgb_color_conversion_transformation_ctl.h"
#else
#include "kis_rgb_to_ks_color_conversion_transformation.h"
#include "kis_ks_to_rgb_color_conversion_transformation.h"
#endif
#include "kis_ks_to_ks_color_conversion_transformation.h"

#include "half.h"

template< int _N_ >
class KisKSF16ColorSpace : public KisKSColorSpace< half, _N_ >
{
    typedef KisKSColorSpace< half, _N_ > parent;

public:

    KisKSF16ColorSpace(KoColorProfile *p) : parent(p) { }
    ~KisKSF16ColorSpace() { }

    KoColorSpace *clone() const {
        return new KisKSF16ColorSpace<_N_>(parent::profile()->clone());
    }

};

template< int _N_ >
class KisKSF16ColorSpaceFactory : public KisKSColorSpaceFactory<half, _N_>
{
public:

    QList<KoColorConversionTransformationFactory*> colorConversionLinks() const {
        QList<KoColorConversionTransformationFactory*> list;

        // RGB to KS and vice versa
        KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
        QString csid = KisKSF16ColorSpace<_N_>::ColorSpaceId().id();
        foreach(const KoColorProfile *p, f->profilesFor(csid)) {
#ifdef HAVE_OPENCTL
            list.append(new KisCtlRGBToKSColorConversionTransformationFactory<half, _N_>(p->name()));
            list.append(new KisKSToCtlRGBColorConversionTransformationFactory<half, _N_>(p->name()));
#else
            list.append(new KisRGBToKSColorConversionTransformationFactory<half, _N_>(p->name()));
            list.append(new KisKSToRGBColorConversionTransformationFactory<half, _N_>(p->name()));
#endif
        }
        /*
                    // From F32
                    list.append(new KisKSToKSColorConversionTransformationFactory<float,half,_N_>);

                    // Self to self (profile change)
                    list.append(new KisKSToKSColorConversionTransformationFactory<half,half,_N_>);
        */
        return list;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const {

        Q_ASSERT(p);
        return new KisKSF16ColorSpace<_N_>(p->clone());
    }

};

#endif // KIS_KSF16_COLORSPACE_H_
