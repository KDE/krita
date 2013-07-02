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


#include "kis_animation_factory.h"

#include <QStringList>
#include <QDir>

#include <kcomponentdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kparts/plugin.h>
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
#include <flake/kis_shape_selection.h>
#include "kis_animation_doc.h"
#include "kis_animation_part.h"
#include "kis_animator_aboutdata.h"
#include <kisexiv2/kis_exiv2.h>

KAboutData* KisAnimationFactory::s_aboutData = 0;
KComponentData* KisAnimationFactory::s_instance = 0;

KisAnimationFactory::KisAnimationFactory(QObject *parent) : KPluginFactory(*aboutData(), parent)
{
    (void)componentData();
}

KisAnimationFactory::~KisAnimationFactory(){
    delete s_aboutData;
    s_aboutData = 0;
    delete s_instance;
    s_instance = 0;
}

QObject* KisAnimationFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword){
    Q_UNUSED( args );
    Q_UNUSED( keyword);

    KisAnimationDoc* doc = new KisAnimationDoc();
    return doc->documentPart();
}

KAboutData* KisAnimationFactory::aboutData(){
    if(!s_aboutData){
        s_aboutData = newKritaAnimatorAboutData();
    }

    return s_aboutData;
}

const KComponentData &KisAnimationFactory::componentData(){
    if(!s_instance){
        if(s_aboutData)
            s_instance = new KComponentData(s_aboutData);
        else
            s_instance = new KComponentData(newKritaAnimatorAboutData());
        Q_CHECK_PTR(s_instance);

        KisExiv2::initialize();

        KoShapeRegistry* r = KoShapeRegistry::instance();
        r->add(new KisShapeSelectionFactory());

        KisFilterRegistry::instance();
        KisGeneratorRegistry::instance();
        KisPaintOpRegistry::instance();

        KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Tool"),
                                         QString::fromLatin1("[X-Krita-Version] == 28"));

        KoPluginLoader::PluginsConfig config;
        config.blacklist = "DockerPluginsDisabled";
        config.group = "krita";
        KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Dock"),
                                         QString::fromLatin1("[X-Krita-Version] == 28"));

        s_instance->dirs()->addResourceType("krita_template", "data", "krita/templates");

        s_instance->dirs()->addResourceType("kis_pics", "data", "krita/pics/");

        s_instance->dirs()->addResourceType("kis_images", "data", "krita/images/");

        s_instance->dirs()->addResourceType("icc_profiles", 0, "krita/profiles/");

        s_instance->dirs()->addResourceType("kis_shaders", "data", "krita/shaders/");

        s_instance->dirs()->addResourceType("kis_backgrounds", "data", "krita/backgrounds/");

        KIconLoader::global()->addAppDir("calligra");
    }

    return *s_instance;
}

#include "kis_animation_factory.moc"
