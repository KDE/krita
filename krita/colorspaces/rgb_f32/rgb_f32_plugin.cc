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
#include <KoColorSpaceRegistry.h>
#include <kis_basic_histogram_producers.h>

#include "rgb_f32_plugin.h"
#include "kis_rgb_f32_colorspace.h"

typedef KGenericFactory<RGBF32Plugin> RGBF32PluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_rgb_f32_plugin, RGBF32PluginFactory( "krita" ) )


RGBF32Plugin::RGBF32Plugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    setInstance(RGBF32PluginFactory::instance());

    if ( parent->inherits("KoColorSpaceRegistry") )
    {
	KoColorSpaceRegistry * f = dynamic_cast<KoColorSpaceRegistry*>(parent);

        KoColorSpace * colorSpaceRGBF32  = new KisRgbF32ColorSpace(f, 0);

        KoColorSpaceFactory * csf  = new KisRgbF32ColorSpaceFactory();
        f->add(csf);

        KisHistogramProducerFactoryRegistry::instance()->add(
            new KisBasicHistogramProducerFactory<KisBasicF32HistogramProducer>
            (KoID("RGBF32HISTO", i18n("Float32 Histogram")), colorSpaceRGBF32) );
    }

}

RGBF32Plugin::~RGBF32Plugin()
{
}

#include "rgb_f32_plugin.moc"
