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

#include "rgb_float_hdr.h"
#include "config-openexr.h"

#include <klocale.h>
#include <kgenericfactory.h>

#include "KoColorSpaceRegistry.h"
#include "KoCtlColorProfile.h"
#include "KoBasicHistogramProducers.h"

#include "kis_rgb_f16_hdr_colorspace.h"
#include "kis_rgb_f32_hdr_colorspace.h"

typedef KGenericFactory<RGBFloatHDRPlugin> RGBFloatHDRPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritargbfloathdrplugin, RGBFloatHDRPluginFactory( "krita" ) )


RGBFloatHDRPlugin::RGBFloatHDRPlugin(QObject *parent, const QStringList &)
    : QObject(parent)
{
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();
    // Register F16 HDR colorspace
#if 0
     //def HAVE_OPENEXR
    {
        KoColorSpaceFactory *csf  = new KisRgbF16HDRColorSpaceFactory();
        const KoCtlColorProfile* profile = static_cast<const KoCtlColorProfile*>(f->profileByName( csf->defaultProfile() ) );
        KoColorSpace * colorSpaceRGBF16  = new KisRgbF16HDRColorSpace(profile);
        Q_CHECK_PTR(colorSpaceRGBF16);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
                    new KoBasicHistogramProducerFactory<KoBasicF16HistogramProducer>
                    (KoID("RGBF16HISTO", i18n("Float16 Half Histogram")), colorSpaceRGBF16) );
    }
#endif
    // Register F32 HDR colorspace
    {
        KoColorSpaceFactory *csf  = new KisRgbF32HDRColorSpaceFactory();
        const KoCtlColorProfile* profile = static_cast<const KoCtlColorProfile*>(f->profileByName( csf->defaultProfile() ) );
        kDebug() << csf->defaultProfile();
        Q_ASSERT(profile);
        KoColorSpace * colorSpaceRGBF32  = new KisRgbF32HDRColorSpace(profile);
        Q_CHECK_PTR(colorSpaceRGBF32);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
                    new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
                    (KoID("RGBF32HISTO", i18n("Float32 Half Histogram")), colorSpaceRGBF32) );
    }

}

RGBFloatHDRPlugin::~RGBFloatHDRPlugin()
{
}

#include "rgb_float_hdr.moc"
