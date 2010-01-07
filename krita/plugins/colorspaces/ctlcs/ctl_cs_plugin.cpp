/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "ctl_cs_plugin.h"

#include <QMutex>

#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>
#include <KoCtlColorProfile.h>
#include <kstandarddirs.h>

#include <OpenCTL/ModulesManager.h>
#include "kis_debug.h"
#include "KoCtlColorSpaceInfo.h"
#include "KoCtlColorSpaceFactory.h"
#include "KoCtlMutex.h"

QMutex* ctlMutex = 0;

#include <OpenCTL/Template.h>
#include "KoCtlColorTransformationFactory.h"
#include <KoColorTransformationFactoryRegistry.h>

K_PLUGIN_FACTORY(CTLCSPluginPluginFactory, registerPlugin<CTLCSPlugin>();)
K_EXPORT_PLUGIN(CTLCSPluginPluginFactory("krita"))


CTLCSPlugin::CTLCSPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    Q_ASSERT(ctlMutex == 0);
    ctlMutex = new QMutex;
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();
    {
        // Set PigmentCMS's ctl module directory
        QStringList ctlModulesDirs = KGlobal::mainComponent().dirs()->findDirs("data", "pigmentcms/ctlmodules/");
        dbgPlugins << ctlModulesDirs;
        foreach(const QString & dir, ctlModulesDirs) {
            dbgPlugins << "Append : " << dir << " to the list of CTL modules";
            OpenCTL::ModulesManager::instance()->addDirectory(dir.toAscii().data());
        }

        QStringList ctlTemplatesDirs = KGlobal::mainComponent().dirs()->findDirs("data", "pigmentcms/ctltemplates/");
        dbgPlugins << ctlTemplatesDirs;
        foreach(const QString & dir, ctlTemplatesDirs) {
            dbgPlugins << "Append : " << dir << " to the list of CTL modules";
            OpenCTL::Template::addIncludeDirectory(dir.toAscii().data());
        }


        // Load CTL Profiles
        KGlobal::mainComponent().dirs()->addResourceType("ctl_profiles", "data", "pigmentcms/ctlprofiles/");
        QStringList ctlprofileFilenames;
        ctlprofileFilenames += KGlobal::mainComponent().dirs()->findAllResources("ctl_profiles", "*.ctlp",  KStandardDirs::Recursive);
        dbgPlugins << "There are " << ctlprofileFilenames.size() << " CTL profiles";
        if (!ctlprofileFilenames.empty()) {
            KoColorProfile* profile = 0;
            for (QStringList::Iterator it = ctlprofileFilenames.begin(); it != ctlprofileFilenames.end(); ++it) {
                dbgPlugins << "Load profile : " << *it;
                profile = new KoCtlColorProfile(*it);
                profile->load();
                if (profile->valid()) {
                    dbgPlugins << "Valid profile : " << profile->name();
                    f->addProfile(profile);
                } else {
                    dbgPlugins << "Invalid profile : " << profile->name();
                    delete profile;
                }
            }
        }

        // Load CTL Color spaces
        KGlobal::mainComponent().dirs()->addResourceType("ctl_colorspaces", "data", "pigmentcms/ctlcolorspaces/");
        QStringList ctlcolorspacesFilenames;
        ctlcolorspacesFilenames += KGlobal::mainComponent().dirs()->findAllResources("ctl_colorspaces", "*.ctlcs",  KStandardDirs::Recursive);
        dbgPlugins << "There are " << ctlcolorspacesFilenames.size() << " CTL colorspaces";
        if (!ctlcolorspacesFilenames.empty()) {
            for (QStringList::Iterator it = ctlcolorspacesFilenames.begin(); it != ctlcolorspacesFilenames.end(); ++it) {
                dbgPlugins << "Load colorspace : " << *it;
                KoCtlColorSpaceInfo* info = new KoCtlColorSpaceInfo(*it);
                if (info->load()) {
                    f->add(new KoCtlColorSpaceFactory(info));
                } else {
                    dbgPlugins << "Invalid color space : " << *it;
                    delete info;
                }
            }
        }

        // Load CTL Extensions
        KGlobal::mainComponent().dirs()->addResourceType("ctl_colortransformations", "data", "pigmentcms/ctlcolortransformations/");
        QStringList ctlextensionsFilenames;
        ctlextensionsFilenames += KGlobal::mainComponent().dirs()->findAllResources("ctl_colortransformations", "*.ctlt",  KStandardDirs::Recursive);
        dbgPlugins << "There are " << ctlextensionsFilenames.size() << " CTL color transformations";
        if (!ctlextensionsFilenames.empty()) {
            for (QStringList::Iterator it = ctlextensionsFilenames.begin(); it != ctlextensionsFilenames.end(); ++it) {
                OpenCTL::Template* ctltemplate = new OpenCTL::Template;
                ctltemplate->loadFromFile(it->toAscii().data());
                ctltemplate->compile();
                if (ctltemplate->isCompiled()) {
                    KoCtlColorTransformationFactory* factory = new KoCtlColorTransformationFactory(ctltemplate);
                    dbgPlugins << "Valid ctl color transformations template: " << factory->id();
                    KoColorTransformationFactoryRegistry::addColorTransformationFactory( factory );
                } else {
                    dbgPlugins << "Invalid ctl color transformations template: " << *it;
                    delete ctltemplate;
                }
            }
        }


#if 0
        KisLmsAF32ColorSpaceFactory * csf  = new KisLmsAF32ColorSpaceFactory();
        f->add(csf);
        const KoCtlColorProfile* profile = static_cast<const KoCtlColorProfile*>(f->profileByName(csf->defaultProfile()));
        Q_ASSERT(profile);
        KoColorSpace * colorSpaceLMSF32  = new KisLmsAF32ColorSpace(profile);

        KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("LMSF32HISTO", i18n("Float32 Histogram")), colorSpaceLMSF32));
#endif
    }

}

CTLCSPlugin::~CTLCSPlugin()
{
}

#include "ctl_cs_plugin.moc"
