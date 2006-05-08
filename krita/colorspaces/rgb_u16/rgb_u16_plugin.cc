/*
* rgb_u16_plugin.cc -- Part of Krita
*
* Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
* Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include <kinstance.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kis_debug_areas.h>
#include <kis_colorspace_factory_registry.h>

#include "rgb_u16_plugin.h"
#include "kis_rgb_u16_colorspace.h"
#include "kis_basic_histogram_producers.h"

typedef KGenericFactory<RGBU16Plugin> RGBU16PluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_rgb_u16_plugin, RGBU16PluginFactory( "krita" ) )


RGBU16Plugin::RGBU16Plugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    setInstance(RGBU16PluginFactory::instance());

    if ( parent->inherits("KisColorSpaceFactoryRegistry") )
    {
        KisColorSpaceFactoryRegistry * f = dynamic_cast<KisColorSpaceFactoryRegistry *>( parent );

        KisColorSpace * colorSpaceRGBU16 = new KisRgbU16ColorSpace(f, 0);
        KisColorSpaceFactory * csFactory = new KisRgbU16ColorSpaceFactory();
        f->add( csFactory );

        KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU16HistogramProducer>
                (KisID("RGB16HISTO", i18n("RGB16 Histogram")), colorSpaceRGBU16) );
    }

}

RGBU16Plugin::~RGBU16Plugin()
{
}

#include "rgb_u16_plugin.moc"
