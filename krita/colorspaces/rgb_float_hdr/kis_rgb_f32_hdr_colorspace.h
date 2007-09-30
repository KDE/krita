/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_RGB_F32_HDR_COLORSPACE_H_
#define KIS_RGB_F32_HDR_COLORSPACE_H_

#include <klocale.h>

#include "kis_rgb_float_hdr_colorspace.h"
#include "krita_rgbf32_export.h"

#include "KoColorSpaceTraits.h"
#include <KoColorModelStandardIds.h>
#include "KoScaleColorConversionTransformation.h"

typedef KoRgbTraits<float> RgbF32Traits;

class KRITA_RGBF32_EXPORT KisRgbF32HDRColorSpace : public KisRgbFloatHDRColorSpace<RgbF32Traits>
{
public:
    KisRgbF32HDRColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);

    virtual KoID colorModelId() const { return RGBAColorModelID; }
    virtual KoID colorDepthId() const { return Float32BitsColorDepthID; }
    /**
     * The ID that identifies this colorspace. Pass this as the colorSpaceId parameter 
     * to the KoColorSpaceRegistry::colorSpace() functions to obtain this colorspace.
     * This is the value that the member function id() returns.
     */
    static QString colorSpaceId();
};

class KRITA_RGBF32_EXPORT KisRgbF32HDRColorSpaceFactory : public KisRgbFloatHDRColorSpaceFactory
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
    
    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p) const { return new KisRgbF32HDRColorSpace(parent, p); }
    virtual QList<KoColorConversionTransformationFactory*> colorConversionLinks() const
    {
        QList<KoColorConversionTransformationFactory*> list;
        // Conversion to RGB Float 32bit
        list.append(new KoScaleColorConversionTransformationFactory< KoRgbTraits<quint8>, RgbF32Traits >( RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), Float32BitsColorDepthID.id() ) );
        list.append(new KoScaleColorConversionTransformationFactory< KoRgbU16Traits, RgbF32Traits >( RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), Float32BitsColorDepthID.id() ) );
        // Conversion from RGB Float 32bit
        list.append(new KoScaleColorConversionTransformationFactory< RgbF32Traits, KoRgbTraits<quint8> >( RGBAColorModelID.id(), Float32BitsColorDepthID.id(), Integer8BitsColorDepthID.id() ) );
        list.append(new KoScaleColorConversionTransformationFactory< RgbF32Traits, KoRgbU16Traits  >( RGBAColorModelID.id(), Float32BitsColorDepthID.id(), Integer16BitsColorDepthID.id() ) );
        
        return list;
    }
    virtual KoColorConversionTransformationFactory* createICCColorConversionTransformationFactory(QString _colorModelId, QString _colorDepthId) const
    {
        Q_UNUSED(_colorModelId);
        Q_UNUSED(_colorDepthId);
        return 0;
    }
};

#endif

