/*
* rgb_f32_plugin.cc -- Part of Krita
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
*  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kinstance.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kis_debug_areas.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_basic_histogram_producers.h>

#include "rgb_f16half_plugin.h"
#include "kis_rgb_f16half_colorspace.h"

typedef KGenericFactory<RGBF16HalfPlugin> RGBF16HalfPluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_rgb_f16half_plugin, RGBF16HalfPluginFactory( "krita" ) )


RGBF16HalfPlugin::RGBF16HalfPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    setInstance(RGBF16HalfPluginFactory::instance());

    if ( parent->inherits("KisColorSpaceFactoryRegistry") )
    {
        KisColorSpaceFactoryRegistry * f = dynamic_cast<KisColorSpaceFactoryRegistry*>( parent );

        KisColorSpace * colorSpaceRGBF16Half  = new KisRgbF16HalfColorSpace(f, 0);
        KisColorSpaceFactory *csf  = new KisRgbF16HalfColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceRGBF16Half);
        f->add(csf);
        KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicF16HalfHistogramProducer>
                (KisID("RGBF16HALFHISTO", i18n("Float16 Half Histogram")), colorSpaceRGBF16Half) );
    }

}

RGBF16HalfPlugin::~RGBF16HalfPlugin()
{
}

#include "rgb_f16half_plugin.moc"

