/*
 *
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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

#include <kgenericfactory.h>
#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>
#include "xyz_u16_plugin.h"
#include "kis_xyz_u16_colorspace.h"

typedef KGenericFactory<XYZU16Plugin> XYZU16PluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_xyz_u16_plugin, XYZU16PluginFactory( "krita" ) )


XYZU16Plugin::XYZU16Plugin(QObject *parent, const QStringList &)
    : QObject(parent)
{
    
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();

    KoColorSpaceFactory * csf = new KisXyzU16ColorSpaceFactory();
    f->add(csf);
    
    KoColorProfile *xyzProfile = new KoColorProfile(cmsCreateXYZProfile());
    f->addProfile(xyzProfile);
    
    KoColorSpace * colorSpaceXYZU16 = new KisXyzU16ColorSpace(f, KoColorSpaceRegistry::instance()->profileByName(csf->defaultProfile()));
    Q_CHECK_PTR(colorSpaceXYZU16);
    
    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("XYZ16HISTO", i18n("XYZ16 Histogram")), colorSpaceXYZU16) );

}

XYZU16Plugin::~XYZU16Plugin()
{
}

#include "xyz_u16_plugin.moc"
