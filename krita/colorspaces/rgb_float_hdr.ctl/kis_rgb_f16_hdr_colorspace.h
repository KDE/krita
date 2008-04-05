/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KIS_RGB_F16_HDR_COLORSPACE_H_
#define _KIS_RGB_F16_HDR_COLORSPACE_H_

#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>

#include "kis_rgb_float_hdr_colorspace.h"

typedef KoRgbTraits<float> RgbF16Traits;

class KisRgbF16HDRColorSpace : public KisRgbFloatHDRColorSpace<RgbF16Traits>
{
public:
    KisRgbF16HDRColorSpace( const KoCtlColorProfile *p);

    virtual KoID colorModelId() const { return RGBAColorModelID; }
    virtual KoID colorDepthId() const { return Float16BitsColorDepthID; }
   /**
     * The ID that identifies this colorspace. Pass this as the colorSpaceId parameter 
     * to the KoColorSpaceRegistry::colorSpace() functions to obtain this colorspace.
     * This is the value that the member function id() returns.
     */
    static QString colorSpaceId();
    virtual KoColorSpace* clone() const;
};



class KisRgbF16HDRColorSpaceFactory : public KisRgbFloatHDRColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual QString id() const { return KisRgbF16HDRColorSpace::colorSpaceId(); }
    virtual QString name() const { return i18n("RGB (16-bit float/channel) for High Dynamic Range imaging"); }
    virtual KoID colorModelId() const { return RGBAColorModelID; }
    virtual KoID colorDepthId() const { return Float16BitsColorDepthID; }
    virtual bool userVisible() const { return true; }
    
    virtual int referenceDepth() const { return 16; }

    virtual KoColorSpace *createColorSpace( const KoColorProfile * p) const;
    virtual bool profileIsCompatible(const KoColorProfile* profile) const;
};

#endif
