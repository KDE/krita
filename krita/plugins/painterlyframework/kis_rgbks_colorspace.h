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

#ifndef KIS_RGBKS_COLORSPACE_H_
#define KIS_RGBKS_COLORSPACE_H_

#include "KoIncompleteColorSpace.h"
#include "KoColorSpaceTraits.h"
#include "kritapainterlycommon_export.h"

class QString;
class KoColorProfile;

template<typename _channels_type_>
struct KisRGBKSColorSpaceTraits : public KoColorSpaceTrait<_channels_type_, 7, 6> {

    struct Cell {
        struct {
            _channels_type_ absorption;
            _channels_type_ scattering;
        } wavelen[3];
        _channels_type_ alpha;
    };

};

typedef KisRGBKSColorSpaceTraits<float> RGBKSTraits;

class KRITAPAINTERLYCOMMON_EXPORT KisRGBKSColorSpace : public KoIncompleteColorSpace<RGBKSTraits, KoRGB16Fallback>
{
    public:

        ~KisRGBKSColorSpace()
        {
        }

        KisRGBKSColorSpace();
        virtual KoID colorModelId() const;
        virtual KoID colorDepthId() const;

//         KisRGBKSColorSpace(const KisRGBKSColorSpace&);
//         KisRGBKSColorSpace operator=(const KisRGBKSColorSpace&);

    public:

        bool willDegrade(ColorSpaceIndependence) const
        {
            return true;
        }

        bool profileIsCompatible(KoColorProfile *) const
        {
            return false;
        }

        void fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;
        void toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;
};


#endif // KIS_RGBKS_COLORSPACE_H_
