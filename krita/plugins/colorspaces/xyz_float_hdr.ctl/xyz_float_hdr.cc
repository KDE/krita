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

#include "xyz_float_hdr.h"
#include <config-openexr.h>

#include <klocale.h>
#include <kgenericfactory.h>

#include "KoColorSpaceRegistry.h"
#include "KoCtlColorProfile.h"
#include "KoBasicHistogramProducers.h"


#include "kis_xyz_f32_hdr_colorspace.h"

typedef KGenericFactory<XYZFloatHDRPlugin> XYZFloatHDRPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kritaxyzfloathdrplugin, XYZFloatHDRPluginFactory("krita"))


XYZFloatHDRPlugin::XYZFloatHDRPlugin(QObject *parent, const QStringList &)
        : QObject(parent)
{
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();

    // Register F32 HDR colorspace
    {
        KoColorSpaceFactory *csf  = new KisXyzF32HDRColorSpaceFactory();
        const KoCtlColorProfile* profile = static_cast<const KoCtlColorProfile*>(f->profileByName(csf->defaultProfile()));
        KoColorSpace * colorSpaceXYZF32  = new KisXyzF32HDRColorSpace(profile);
        Q_CHECK_PTR(colorSpaceXYZF32);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("XYZF32HISTO", i18n("Float32 Half Histogram")), colorSpaceXYZF32));
    }

}

XYZFloatHDRPlugin::~XYZFloatHDRPlugin()
{
}

#include "xyz_float_hdr.moc"
