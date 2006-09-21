/*
 * ycbcr_u16_plugin.cc -- Part of Krita
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
 *   along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kinstance.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <KoColorSpaceRegistry.h>
#include <kis_basic_histogram_producers.h>
#include "ycbcr_u16_plugin.h"
#include "kis_ycbcr_u16_colorspace.h"

typedef KGenericFactory<YCbCrU16Plugin> YCbCrU16PluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_ycbcr_u16_plugin, YCbCrU16PluginFactory( "krita" ) )


YCbCrU16Plugin::YCbCrU16Plugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    setInstance(YCbCrU16PluginFactory::instance());

    if ( parent->inherits("KoColorSpaceRegistry") )
    {
        KoColorSpaceRegistry * f = dynamic_cast<KoColorSpaceRegistry*>( parent );

        KoColorSpace * colorSpaceYCbCrU16 = new KisYCbCrU16ColorSpace(f, 0);
        KoColorSpaceFactory * csf = new KisYCbCrU16ColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceYCbCrU16);
        f->add(csf);
        KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU16HistogramProducer>
                (KoID("YCbCr16HISTO", i18n("YCbCr16 Histogram")), colorSpaceYCbCrU16) );
    }

}

YCbCrU16Plugin::~YCbCrU16Plugin()
{
}

#include "ycbcr_u16_plugin.moc"
