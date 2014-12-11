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

//#ifdef _MSC_VER // this removes KDEWIN extensions to stdint.h: required by exiv2
//#define KDEWIN_STDINT_H
//#endif

#include "kis_factory2.h"

#include <QStringList>
#include <QDir>

#include <kcomponentdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include <kis_debug.h>

#include "kis_aboutdata.h"

#include "KisPart.h"


KAboutData* KisFactory::s_aboutData = 0;
KComponentData* KisFactory::s_instance = 0;

KisFactory::KisFactory(QObject* parent)
    : KPluginFactory(*aboutData(), parent)
{
    (void)componentData();
}

KisFactory::~KisFactory()
{
    delete s_aboutData;
    s_aboutData = 0;
    delete s_instance;
    s_instance = 0;
}

/**
 * Create the document
 */
QObject* KisFactory::create( const char* /*iface*/, QWidget* /*parentWidget*/, QObject *parent,
                              const QVariantList& args, const QString& keyword )
{
    Q_UNUSED( parent );
    Q_UNUSED( args );
    Q_UNUSED( keyword );

    return KisPart::instance();
}


KAboutData* KisFactory::aboutData()
{
    if (!s_aboutData) {
        s_aboutData = newKritaAboutData();
    }
    return s_aboutData;
}

const KComponentData &KisFactory::componentData()
{
    if (!s_instance) {
        s_instance = new KComponentData(aboutData());
        Q_CHECK_PTR(s_instance);
        s_instance->dirs()->addResourceType("krita_template", "data", "krita/templates");

        // for cursors
        s_instance->dirs()->addResourceType("kis_pics", "data", "krita/pics/");

        // for images in the paintop box
        s_instance->dirs()->addResourceType("kis_images", "data", "krita/images/");

        s_instance->dirs()->addResourceType("icc_profiles", 0, "krita/profiles/");

        s_instance->dirs()->addResourceType("kis_shaders", "data", "krita/shaders/");

        // Tell the iconloader about share/apps/calligra/icons
        KIconLoader::global()->addAppDir("calligra");

        KGlobal::locale()->insertCatalog(s_instance->catalogName());
        // install 'instancename'data resource type
        KGlobal::dirs()->addResourceType(QString(s_instance->componentName() + "data").toUtf8(), "data", s_instance->componentName());

    }

    return *s_instance;
}

const QString KisFactory::componentName()
{
    return "krita";
}

#include "kis_factory2.moc"
