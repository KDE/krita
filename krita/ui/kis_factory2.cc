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
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>
#include <kconfiggroup.h>

#include <KoPluginLoader.h>
#include <KoShapeRegistry.h>

#include <kis_debug.h>
#include <metadata/kis_meta_data_io_backend.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <generator/kis_generator_registry.h>
#include <generator/kis_generator.h>
#include <kis_paintop_registry.h>

#include "kis_aboutdata.h"
#include "flake/kis_shape_selection.h"
#include "kis_doc2.h"
#include "kis_part2.h"

#include "kisexiv2/kis_exiv2.h"

KAboutData* KisFactory2::s_aboutData = 0;
KComponentData* KisFactory2::s_instance = 0;

KisFactory2::KisFactory2(QObject* parent)
    : KPluginFactory(*aboutData(), parent)
{
    (void)componentData();
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
QObject* KisFactory2::create( const char* /*iface*/, QWidget* /*parentWidget*/, QObject *parent,
                              const QVariantList& args, const QString& keyword )
{
    Q_UNUSED( args );
    Q_UNUSED( keyword );

    KisPart2 *part = new KisPart2(parent);
    KisDoc2 *doc = new KisDoc2(part);
    part->setDocument(doc);

    return part;
}


KAboutData* KisFactory2::aboutData()
{
    if (!s_aboutData) {
        s_aboutData = newKritaAboutData();
    }
    return s_aboutData;
}

const KComponentData &KisFactory2::componentData()
{
    if (!s_instance) {
        if (s_aboutData)
            s_instance = new KComponentData(s_aboutData);
        else
            s_instance = new KComponentData(newKritaAboutData());
        Q_CHECK_PTR(s_instance);

        // XXX_EXIV: make the exiv io backends real plugins
        KisExiv2::initialize();

        KoShapeRegistry* r = KoShapeRegistry::instance();
        r->add(new KisShapeSelectionFactory());

        KisFilterRegistry::instance();
        KisGeneratorRegistry::instance();
        KisPaintOpRegistry::instance();

        // Load the krita-specific tools
        KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Tool"),
                                         QString::fromLatin1("[X-Krita-Version] == 5"));

        // Load dockers
        KoPluginLoader::PluginsConfig config;
        config.blacklist = "DockerPluginsDisabled";
        config.group = "krita";
        KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Dock"),
                                         QString::fromLatin1("[X-Krita-Version] == 5"));

        s_instance->dirs()->addResourceType("krita_template", "data", "krita/templates");

        // for cursors
        s_instance->dirs()->addResourceType("kis_pics", "data", "krita/pics/");

        // for images in the paintop box
        s_instance->dirs()->addResourceType("kis_images", "data", "krita/images/");

        s_instance->dirs()->addResourceType("icc_profiles", 0, "krita/profiles/");

        s_instance->dirs()->addResourceType("kis_shaders", "data", "krita/shaders/");

        s_instance->dirs()->addResourceType("kis_backgrounds", "data", "krita/backgrounds/");

        // Tell the iconloader about share/apps/calligra/icons
        KIconLoader::global()->addAppDir("calligra");
    }

    return *s_instance;
}

#include "kis_factory2.moc"
