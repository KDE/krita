/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_STRATEGY_COLORSPACE_RGB_H_
#define KIS_STRATEGY_COLORSPACE_RGB_H_

#include <klocale.h>
#include <KoLcmsColorSpace.h>
#include <KoColorSpaceTraits.h>

typedef KoRgbTraits<quint8>  RgbU8Traits;

const qint32 MAX_CHANNEL_RGB = 3;

class KoRgbU8ColorSpace : public KoLcmsColorSpace<RgbU8Traits>
{

public:

    KoRgbU8ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
    virtual bool willDegrade(ColorSpaceIndependence ) const { return false; }
    virtual KoColorTransformation* createInvertTransformation() const;

    /**
     * The ID that identifies this colorspace. Pass this as the colorSpaceId parameter 
     * to the KoColorSpaceRegistry::colorSpace() functions to obtain this colorspace.
     * This is the value that the member function id() returns.
     */
    static QString colorSpaceId();
};

class KoRgbU8ColorSpaceFactory : public KoLcmsColorSpaceFactory
{

public:

    KoRgbU8ColorSpaceFactory() : KoLcmsColorSpaceFactory(TYPE_BGRA_8,icSigRgbData )
    {}
    virtual QString id() const { return KoRgbU8ColorSpace::colorSpaceId(); }
    virtual QString name() const { return i18n("RGB (8-bit integer/channel)"); }

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p) { return new KoRgbU8ColorSpace(parent, p); }

    virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; }
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_
