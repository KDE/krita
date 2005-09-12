/*
 *  kis_factory.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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
#include <config.h>
#include LCMS_HEADER

#include <qstringlist.h>

#include <kdebug.h>
#include <kinstance.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>

#include "kis_factory.h"
#include "kis_aboutdata.h"
#include "kis_resourceserver.h"
#include "kis_colorspace_registry.h"
#include "kis_paintop_registry.h"
#include "kis_filter_registry.h"
#include "kis_tool_registry.h"
#include "kis_doc.h"

#include "kis_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_gradient.h"
#include "kis_pattern.h"
#include "kis_palette.h"

#include <kogradientmanager.h>


KAboutData* KisFactory::s_aboutData = 0;
KInstance* KisFactory::s_global = 0;
KisResourceServerRegistry* KisFactory::s_rserverRegistry = 0;


KisFactory::KisFactory( QObject* parent, const char* name )
    : KoFactory( parent, name )
{
    s_aboutData = newKritaAboutData();

    (void)global();

    s_rserverRegistry = new KisResourceServerRegistry();

    QStringList fileExtensions;
    fileExtensions << "*.gbr";
    KisResourceServer<KisBrush>* brushServer = new KisResourceServer<KisBrush>("kis_brushes", fileExtensions);
    Q_CHECK_PTR(brushServer);
    s_rserverRegistry -> add( KisID( "BrushServer", ""), brushServer );

    fileExtensions.clear();
    fileExtensions << "*.gih";
    KisResourceServer<KisImagePipeBrush>* imagePipeBrushServer = new KisResourceServer<KisImagePipeBrush>("kis_brushes", fileExtensions);
    Q_CHECK_PTR(imagePipeBrushServer);
    s_rserverRegistry -> add( KisID( "ImagePipeBrushServer", ""), imagePipeBrushServer );

    fileExtensions.clear();
    fileExtensions << "*.pat";
    KisResourceServer<KisPattern>* patternServer = new KisResourceServer<KisPattern>("kis_patterns", fileExtensions);
    Q_CHECK_PTR(patternServer);
    s_rserverRegistry -> add( KisID( "PatternServer", ""), patternServer );

    fileExtensions.clear();
    fileExtensions = KoGradientManager::filters();
    KisResourceServer<KisGradient>* gradientServer = new KisResourceServer<KisGradient>("kis_gradients", fileExtensions);
    Q_CHECK_PTR(gradientServer);
    s_rserverRegistry -> add( KisID( "GradientServer", ""), gradientServer );

    fileExtensions.clear();
    fileExtensions << "*.gpl" << "*.pal" << "*.act";
    KisResourceServer<KisPalette>* paletteServer = new KisResourceServer<KisPalette>("kis_palettes", fileExtensions);
    Q_CHECK_PTR(paletteServer);
    s_rserverRegistry -> add( KisID( "PaletteServer", ""), paletteServer );


    // Load extension modules and plugins
      KisToolRegistry::instance();
    KisPaintOpRegistry::instance();
    KisFilterRegistry::instance();
    KisColorSpaceRegistry::instance();

    // Load all modules: color models, paintops, filters
    KTrader::OfferList offers = KTrader::self() -> query(QString::fromLatin1("Krita/CoreModule"),
                                    QString::fromLatin1("Type == 'Service'"));

    KTrader::OfferList::ConstIterator iter;

    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KParts::ComponentFactory::createInstanceFromService<KParts::Plugin> ( service, this, 0, QStringList(), &errCode);
        if ( plugin )
            kdDebug(DBG_AREA_CORE) << "found plugin " << service -> property("Name").toString() << "\n";
    }


}

KisFactory::~KisFactory()
{
    delete s_rserverRegistry;
    s_rserverRegistry = 0L;
    delete s_aboutData;
    s_aboutData = 0L;
    delete s_global;
    s_global = 0L;
}

/**
 * Create the document
 */
KParts::Part* KisFactory::createPartObject( QWidget *parentWidget,
                        const char *widgetName, QObject* parent,
                        const char* name, const char* classname, const QStringList & )
{
    bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

    KisDoc *doc = new KisDoc( parentWidget,
                  widgetName, parent, name, !bWantKoDocument );
    Q_CHECK_PTR(doc);

    if ( !bWantKoDocument )
        doc->setReadWrite( false );

    return doc;
}

KInstance* KisFactory::global()
{
    if ( !s_global )
    {
        s_global = new KInstance(s_aboutData);
        Q_CHECK_PTR(s_global);
    
        s_global -> dirs() -> addResourceType("krita_template",
                         KStandardDirs::kde_default("data") + "krita/templates");

        s_global -> dirs() -> addResourceType("kis",
                          KStandardDirs::kde_default("data") + "krita/");

        s_global -> dirs() -> addResourceType("kis_images",
                          KStandardDirs::kde_default("data") + "krita/images/");

        s_global -> dirs() -> addResourceType("kis_brushes",
                          KStandardDirs::kde_default("data") + "krita/brushes/");

        s_global -> dirs() -> addResourceType("kis_patterns",
                          KStandardDirs::kde_default("data") + "krita/patterns/");

        s_global -> dirs() -> addResourceType("kis_gradients",
                          KStandardDirs::kde_default("data") + "krita/gradients/");

        s_global -> dirs() -> addResourceType("kis_pics",
                          KStandardDirs::kde_default("data") + "krita/pics/");

        s_global -> dirs() -> addResourceType("toolbars",
                          KStandardDirs::kde_default("data") + "koffice/toolbar/");

        s_global -> dirs() -> addResourceType("kis_profiles",
                          KStandardDirs::kde_default("data") + "krita/profiles/");

        s_global -> dirs() -> addResourceType("kis_profiles", "/usr/share/color/icc/");

        s_global -> dirs() -> addResourceType("kis_palettes",
                          KStandardDirs::kde_default("data") + "krita/palettes/");

        // Tell the iconloader about share/apps/koffice/icons
        s_global -> iconLoader() -> addAppDir("koffice");
    }

    return s_global;
}

KAboutData* KisFactory::aboutData()
{
    return s_aboutData;
}

KisResourceServerRegistry* KisFactory::rServerRegistry()
{
    return s_rserverRegistry;
}

#include "kis_factory.moc"
