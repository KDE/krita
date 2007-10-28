/*
 * rgb_plugin.cc -- Part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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
#include "rgb_float_hdr.h"

#include "config-openexr.h"

#include <klocale.h>
#include <kgenericfactory.h>

#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>
#include <KoChromaticities.h>

#include "kis_rgb_f32_hdr_colorspace.h"

#ifdef HAVE_OPENEXR
#include "kis_rgb_f16_hdr_colorspace.h"
#endif

typedef KGenericFactory<RGBFloatHDRPlugin> RGBFloatHDRPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritargbfloathdrplugin, RGBFloatHDRPluginFactory( "krita" ) )


RGBFloatHDRPlugin::RGBFloatHDRPlugin(QObject *parent, const QStringList &)
    : QObject(parent)
{
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();

    // Create default profile for rgb float hdr colorspaces.
    KoRGBChromaticities chromaticities;

    chromaticities.primaries.Red.x = 0.6400f;
    chromaticities.primaries.Red.y = 0.3300f;
    chromaticities.primaries.Red.Y = 1.0f;
    chromaticities.primaries.Green.x = 0.3000f;
    chromaticities.primaries.Green.y = 0.6000f;
    chromaticities.primaries.Green.Y = 1.0f;
    chromaticities.primaries.Blue.x = 0.1500f;
    chromaticities.primaries.Blue.y = 0.0600f;
    chromaticities.primaries.Blue.Y = 1.0f;
    chromaticities.whitePoint.x = 0.3127f;
    chromaticities.whitePoint.y = 0.3290f;
    chromaticities.whitePoint.Y = 1.0f;

    const double gamma = 1.0;

    KoIccColorProfile *profile = new KoIccColorProfile(chromaticities, gamma, "lcms virtual RGB profile - Rec. 709 Linear");

    f->addProfile(profile);

#ifdef HAVE_OPENEXR
    // Register F16 HDR colorspace
    {
        KoColorSpace * colorSpaceRGBF16Half  = new KisRgbF16HDRColorSpace(f, profile);
        KoColorSpaceFactory *csf  = new KisRgbF16HDRColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceRGBF16Half);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
                    new KoBasicHistogramProducerFactory<KoBasicF16HalfHistogramProducer>
                    (KoID("RGBF16HALFHISTO", i18n("Float16 Half Histogram")), colorSpaceRGBF16Half) );
    }
#endif
    // Register F32 HDR colorspace
    {
        KoColorSpace * colorSpaceRGBF32  = new KisRgbF32HDRColorSpace(f, profile);
        KoColorSpaceFactory *csf  = new KisRgbF32HDRColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceRGBF32);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
                    new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
                    (KoID("RGBF32HALFHISTO", i18n("Float32 Half Histogram")), colorSpaceRGBF32) );
    }
}

RGBFloatHDRPlugin::~RGBFloatHDRPlugin()
{
}

#include "rgb_float_hdr.moc"
