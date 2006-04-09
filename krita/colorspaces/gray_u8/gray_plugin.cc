/*
 * gray_plugin.cc -- Part of Krita
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

#include "gray_plugin.h"
#include "kis_gray_colorspace.h"

typedef KGenericFactory<GrayPlugin> GrayPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritagrayplugin, GrayPluginFactory( "kritacore" ) )


GrayPlugin::GrayPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent)
{
    setObjectName(name);
    setInstance(GrayPluginFactory::instance());

    // This is not a gui plugin; only load it when the doc is created.
    if ( parent->inherits("KisColorSpaceFactoryRegistry") )
    {

        KisColorSpaceFactoryRegistry * f = dynamic_cast<KisColorSpaceFactoryRegistry*>( parent );

        // .22 gamma grayscale or something like that. Taken from the lcms tutorial...
        LPGAMMATABLE Gamma = cmsBuildGamma(256, 2.2);
        cmsHPROFILE hProfile = cmsCreateGrayProfile(cmsD50_xyY(), Gamma);
        cmsFreeGamma(Gamma);
        KisProfile *defProfile = new KisProfile(hProfile);

        f->addProfile(defProfile);

        KisColorSpace * colorSpaceGrayA = new KisGrayColorSpace(f, 0);

        KisColorSpaceFactory * csf = new KisGrayColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceGrayA);

        f->add(csf);

        KisHistogramProducerFactoryRegistry::instance()->add(
            new KisBasicHistogramProducerFactory<KisBasicU8HistogramProducer>
            (KisID("GRAYA8HISTO", i18n("GRAY/Alpha8 Histogram")), colorSpaceGrayA) );
    }

}

GrayPlugin::~GrayPlugin()
{
}

#include "gray_plugin.moc"
