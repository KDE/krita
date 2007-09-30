/*
 *  Copyright (c) 2004-2006 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_STRATEGY_COLORSPACE_GRAYSCALE_U16_H_
#define KIS_STRATEGY_COLORSPACE_GRAYSCALE_U16_H_
#include <QColor>

#include <klocale.h>
#include <krita_gray_u16_export.h>
#include "KoLcmsColorSpace.h"
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>

typedef KoColorSpaceTrait<quint16, 1, -1> GrayU16Traits;

class KRITA_GRAY_U16_EXPORT KisGrayU16ColorSpace : public KoLcmsColorSpace<GrayU16Traits>
{
    public:
        KisGrayU16ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence ) const { return false; }
        virtual KoID colorModelId() const { return GrayColorModelID; }
        virtual KoID colorDepthId() const { return Integer16BitsColorDepthID; }
};

class KisGrayU16ColorSpaceFactory : public KoLcmsColorSpaceFactory
{
public:
    KisGrayU16ColorSpaceFactory() : KoLcmsColorSpaceFactory(TYPE_GRAY_16, icSigGrayData)
    {}
    virtual QString id() const { return "GRAY16"; }
    virtual QString name() const { return i18n("Grayscale (16-bit integer/channel)"); }
    virtual KoID colorModelId() const { return GrayColorModelID; }
    virtual KoID colorDepthId() const { return Integer16BitsColorDepthID; }
    virtual int referenceDepth() const { return 16; }

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) const { return new KisGrayU16ColorSpace(parent, p); }

    virtual QString defaultProfile() const { return "gray built-in - (lcms internal)"; }
};

#endif // KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
