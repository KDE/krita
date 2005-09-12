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
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <kinstance.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kis_global.h>
#include <kis_colorspace_registry.h>
#include <kis_factory.h>
#include <kis_basic_histogram_producers.h>

#include "rgb_f32_plugin.h"
#include "kis_rgb_f32_colorspace.h"

typedef KGenericFactory<RGBF32Plugin> RGBF32PluginFactory;
K_EXPORT_COMPONENT_FACTORY( krita_rgb_f32_plugin, RGBF32PluginFactory( "krita" ) )


RGBF32Plugin::RGBF32Plugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
        setInstance(RGBF32PluginFactory::instance());

    kdDebug(DBG_AREA_PLUGINS) << "32-bit float RGB Color model plugin. Class: "
        << className()
        << ", Parent: "
        << parent -> className()
        << "\n";

    if ( parent->inherits("KisFactory") )
    {
        KisColorSpace * colorSpaceRGBF32  = new KisF32RgbColorSpace();
        Q_CHECK_PTR(colorSpaceRGBF32);
        KisColorSpaceRegistry::instance() -> add(colorSpaceRGBF32);
        KisHistogramProducerFactoryRegistry::instance() -> add(
                new KisBasicHistogramProducerFactory<KisBasicF32HistogramProducer>
                (KisID("RGBF32HISTO", i18n("Float32 Histogram")), colorSpaceRGBF32) );
    }

}

RGBF32Plugin::~RGBF32Plugin()
{
}

#include "rgb_f32_plugin.moc"
