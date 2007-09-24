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

#ifndef _KIS_XYZ_F32_HDR_COLORSPACE_H_
#define _KIS_XYZ_F32_HDR_COLORSPACE_H_

#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>

#include "kis_xyz_hdr_colorspace.h"

typedef KoXyzTraits<float> XyzF32Traits;

class KisXyzF32HDRColorSpace : public KisXyzFloatHDRColorSpace<XyzF32Traits>
{
public:
    KisXyzF32HDRColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);

    /**
     * The ID that identifies this colorspace. Pass this as the colorSpaceId parameter 
     * to the KoColorSpaceRegistry::colorSpace() functions to obtain this colorspace.
     * This is the value that the member function id() returns.
     */
    static QString colorSpaceId();
};



class KisXyzF32HDRColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual QString id() const { return KisXyzF32HDRColorSpace::colorSpaceId(); }
    virtual QString name() const { return i18n("XYZ (32-bit float/channel) for High Dynamic Range imaging"); }
    virtual KoID colorModelId() const { return XYZAColorModelID; }
    virtual KoID colorDepthId() const { return Float32BitsColorDepthID; }
    
    virtual bool isIcc() const { return false; }
    virtual bool isHdr() const { return true; }
    virtual QList<KoColorConversionLink> colorConversionLinks() const;
    virtual int depth() const { return 32; }

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p) const { return new KisXyzF32HDRColorSpace(parent, p); }
    virtual bool profileIsCompatible(KoColorProfile* profile) const
    {
        return profile == 0;
    }
    QString defaultProfile() const { return ""; }
};

#endif
