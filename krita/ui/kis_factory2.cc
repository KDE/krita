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
#include <lcms.h>

#include <QStringList>
#include <QDir>

#include <kdebug.h>
#include <kinstance.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>

#include "kis_aboutdata.h"

#include "kis_doc2.h"
#include "kis_factory2.h"

KAboutData* KisFactory2::s_aboutData = 0;
KInstance* KisFactory2::s_instance = 0;



KisFactory2::KisFactory2( QObject* parent )
    : KoFactory( parent )
{
    s_aboutData = newKritaAboutData();

    (void)instance();
}

KisFactory2::~KisFactory2()
{
    delete s_aboutData;
    s_aboutData = 0;
    delete s_instance;
    s_instance = 0;
}

/**
 * Create the document
 */
KParts::Part* KisFactory2::createPartObject( QWidget *parentWidget,
                        QObject* parent,
                        const char* classname, const QStringList & )
{
    bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

    KisDoc2 *doc = new KisDoc2( parentWidget, parent, !bWantKoDocument );
    Q_CHECK_PTR(doc);

    if ( !bWantKoDocument )
        doc->setReadWrite( false );

    return doc;
}


KAboutData* KisFactory2::aboutData()
{
    return s_aboutData;
}

KInstance* KisFactory2::instance()
{
    QString homedir = getenv("HOME");

    if ( !s_instance )
    {
        s_instance = new KInstance(s_aboutData);
        Q_CHECK_PTR(s_instance);

        s_instance->dirs()->addResourceType("krita_template", KStandardDirs::kde_default("data") + "krita/templates");

        // XXX: Are these obsolete?
        s_instance->dirs()->addResourceType("kis", KStandardDirs::kde_default("data") + "krita/");
        s_instance->dirs()->addResourceType("kis_pics", KStandardDirs::kde_default("data") + "krita/pics/");
        s_instance->dirs()->addResourceType("kis_images", KStandardDirs::kde_default("data") + "krita/images/");
        s_instance->dirs()->addResourceType("toolbars", KStandardDirs::kde_default("data") + "koffice/toolbar/");

        // Create spec

        s_instance->dirs()->addResourceType("kis_brushes", KStandardDirs::kde_default("data") + "krita/brushes/");
        s_instance->dirs()->addResourceDir("kis_brushes", "/usr/share/create/brushes/gimp");
        s_instance->dirs()->addResourceDir("kis_brushes", QDir::homePath() + QString("/.create/brushes/gimp"));

        s_instance->dirs()->addResourceType("kis_patterns", KStandardDirs::kde_default("data") + "krita/patterns/");
        s_instance->dirs()->addResourceDir("kis_patterns", "/usr/share/create/patterns/gimp");
        s_instance->dirs()->addResourceDir("kis_patterns", QDir::homePath() + QString("/.create/patterns/gimp"));

        s_instance->dirs()->addResourceType("kis_gradients", KStandardDirs::kde_default("data") + "krita/gradients/");
        s_instance->dirs()->addResourceDir("kis_gradients", "/usr/share/create/gradients/gimp");
        s_instance->dirs()->addResourceDir("kis_gradients", QDir::homePath() + QString("/.create/gradients/gimp"));

        s_instance->dirs()->addResourceType("kis_profiles", KStandardDirs::kde_default("data") + "krita/profiles/");
        s_instance->dirs()->addResourceDir("kis_profiles", "/usr/share/color/icc");
        s_instance->dirs()->addResourceDir("kis_profiles", QDir::homePath() + QString("/.icc"));

        s_instance->dirs()->addResourceType("kis_palettes", KStandardDirs::kde_default("data") + "krita/palettes/");
        s_instance->dirs()->addResourceDir("kis_palettes", "/usr/share/create/swatches");
        s_instance->dirs()->addResourceDir("kis_palettes", QDir::homePath() + QString("/.create/swatches"));

        // Tell the iconloader about share/apps/koffice/icons
        s_instance->iconLoader()->addAppDir("koffice");


        // Load the Krita-specific tools. XXX: Move the krita tools to
        // flake tools once the interaction design has been sorted
        // out.
        KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("Krita/Tool"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Krita-Version] == 3)"));

        KService::List::ConstIterator iter;
        for(iter = offers.begin(); iter != offers.end(); ++iter)
        {
            KService::Ptr service = *iter;
            int errCode = 0;
            KParts::Plugin* plugin =
                KService::createInstance<KParts::Plugin> ( service, 0, QStringList(), &errCode);
            if ( plugin )
                kDebug(30008) <<"found plugin '"<< service->name() << "'" << endl;
            else {
                if( errCode == KLibLoader::ErrNoLibrary)
                {
                    kWarning(30008) <<"loading plugin '" << service->name() <<
                        "' failed, "<< KLibLoader::errorString( errCode ) << " ("<< errCode << ")" << endl;
                }
            }

        }

    }

    return s_instance;
}

#include "kis_factory2.moc"
