/*
* rgb_plugin.cc -- Part of Krita
*
* Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kis_debug_areas.h>

#include <kis_debug_areas.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_basic_histogram_producers.h>

#include "rgb_plugin.h"
#include "kis_rgb_colorspace.h"

typedef KGenericFactory<RGBPlugin> RGBPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritargbplugin, RGBPluginFactory( "krita" ) )


RGBPlugin::RGBPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent)
{
    setObjectName(name);
    setInstance(RGBPluginFactory::instance());

    if ( parent->inherits("KisColorSpaceFactoryRegistry") )
    {
	KisColorSpaceFactoryRegistry * f = dynamic_cast<KisColorSpaceFactoryRegistry*>(parent);

        KisProfile *defProfile = new KisProfile(cmsCreate_sRGBProfile());
        f->addProfile(defProfile);


        KisColorSpaceFactory * csFactory = new KisRgbColorSpaceFactory();
        f->add(csFactory);

        KisColorSpace * colorSpaceRGBA = new KisRgbColorSpace(f, 0);
        KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU8HistogramProducer>
                (KisID("RGB8HISTO", i18n("RGB8 Histogram")), colorSpaceRGBA) );
    }

}

RGBPlugin::~RGBPlugin()
{
}

#include "rgb_plugin.moc"
