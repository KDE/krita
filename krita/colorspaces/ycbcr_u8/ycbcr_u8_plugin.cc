/*
 * ycbcr_u8_plugin.cc -- Part of Krita
 *
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

#include "ycbcr_u8_plugin.h"

#include <kinstance.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kis_debug_areas.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_basic_histogram_producers.h>
#include <kis_debug_areas.h>

#include "kis_ycbcr_u8_colorspace.h"

typedef KGenericFactory<YCbCrU8Plugin> YCbCrU8PluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_ycbcr_u16_plugin, YCbCrU8PluginFactory( "krita" ) )


YCbCrU8Plugin::YCbCrU8Plugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    setInstance(YCbCrU8PluginFactory::instance());

    if ( parent->inherits("KisColorSpaceFactoryRegistry") )
    {
        KisColorSpaceFactoryRegistry * f = dynamic_cast<KisColorSpaceFactoryRegistry*>( parent );

        KisColorSpace * colorSpaceYCbCrU8 = new KisYCbCrU8ColorSpace(f, 0);
        KisColorSpaceFactory * csf = new KisYCbCrU8ColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceYCbCrU8);
        f->add(csf);
        KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU16HistogramProducer>
                (KisID("YCBR8HISTO", i18n("YCBR8 Histogram")), colorSpaceYCbCrU8) );
    }

}

YCbCrU8Plugin::~YCbCrU8Plugin()
{
}

#include "ycbcr_u8_plugin.moc"
