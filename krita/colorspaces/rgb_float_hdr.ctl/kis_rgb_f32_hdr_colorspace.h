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

#ifndef _KIS_RGB_F32_HDR_COLORSPACE_H_
#define _KIS_RGB_F32_HDR_COLORSPACE_H_

#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>

#include "kis_rgb_float_hdr_colorspace.h"

typedef KoRgbTraits<float> RgbF32Traits;

class KisRgbF32HDRColorSpace : public KisRgbFloatHDRColorSpace<RgbF32Traits>
{
public:
    KisRgbF32HDRColorSpace( const KoCtlColorProfile *p);

    virtual KoID colorModelId() const { return RGBAColorModelID; }
    virtual KoID colorDepthId() const { return Float32BitsColorDepthID; }
   /**
     * The ID that identifies this colorspace. Pass this as the colorSpaceId parameter 
     * to the KoColorSpaceRegistry::colorSpace() functions to obtain this colorspace.
     * This is the value that the member function id() returns.
     */
    static QString colorSpaceId();
    virtual KoColorSpace* clone() const;
};



class KisRgbF32HDRColorSpaceFactory : public KisRgbFloatHDRColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual QString id() const { return KisRgbF32HDRColorSpace::colorSpaceId(); }
    virtual QString name() const { return i18n("RGB (32-bit float/channel) for High Dynamic Range imaging"); }
    virtual KoID colorModelId() const { return RGBAColorModelID; }
    virtual KoID colorDepthId() const { return Float32BitsColorDepthID; }
    virtual bool userVisible() const { return true; }
    
    virtual int referenceDepth() const { return 32; }

    virtual KoColorSpace *createColorSpace( const KoColorProfile * p) const;
    virtual bool profileIsCompatible(const KoColorProfile* profile) const;
};

#endif
