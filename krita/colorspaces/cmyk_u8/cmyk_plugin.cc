/*
 * cmyk_plugin.cc -- Part of Krita
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

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_debug_areas.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_basic_histogram_producers.h>

#include "cmyk_plugin.h"

#include "kis_cmyk_colorspace.h"

typedef KGenericFactory<CMYKPlugin> CMYKPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritacmykplugin, CMYKPluginFactory( "krita" ) )


CMYKPlugin::CMYKPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent)
{
    setObjectName(name);
    setInstance(CMYKPluginFactory::instance());
    if ( parent->inherits("KisColorSpaceFactoryRegistry") )
    {
        KisColorSpaceFactoryRegistry * f = dynamic_cast<KisColorSpaceFactoryRegistry*>( parent );

        KisColorSpace * colorSpaceCMYK = new KisCmykColorSpace(f, 0);
        KisColorSpaceFactory * csf = new KisCmykColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceCMYK);
        f->add(csf);

        KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU8HistogramProducer>
                (KisID("CMYKHISTO", i18n("CMYK Histogram")), colorSpaceCMYK) );
    }

}

CMYKPlugin::~CMYKPlugin()
{
}

#include "cmyk_plugin.moc"
