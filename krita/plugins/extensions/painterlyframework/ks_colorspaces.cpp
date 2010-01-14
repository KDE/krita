/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#include "ks_colorspaces.h"

#include "kis_illuminant_profile.h"

#include "kis_ks_colorspace.h"
#include "kis_ks_colorspace_engine.h"
#include "kis_ksf32_colorspace.h"

#include <KGenericFactory>
#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>

#ifdef HAVE_OPENEXR
#include "half.h"
#include "kis_ksf16_colorspace.h"
#endif

K_PLUGIN_FACTORY(KSColorSpacesPluginFactory, registerPlugin<KSColorSpacesPlugin>();)
K_EXPORT_PLUGIN(KSColorSpacesPluginFactory("krita"))

KSColorSpacesPlugin::KSColorSpacesPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    QStringList list;
    KoColorSpaceEngineRegistry *e = KoColorSpaceEngineRegistry::instance();
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();

    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.xll",  KStandardDirs::Recursive);

    KisIlluminantProfile *p;
    foreach(const QString & curr, list) {
        p = new KisIlluminantProfile(curr);
        bool r = p->load();
        if (!r) {
            qWarning() << "failed to load profile " << curr;
            delete p;
        } else {
            f->addProfile(p);
        }
    }

    e->add(new KisKSColorSpaceEngine<3>);
    e->add(new KisKSColorSpaceEngine<4>);
    e->add(new KisKSColorSpaceEngine<6>);
    e->add(new KisKSColorSpaceEngine<10>);

    f->add(new KisKSF32ColorSpaceFactory<3>);
    f->add(new KisKSF32ColorSpaceFactory<4>);
    f->add(new KisKSF32ColorSpaceFactory<6>);
    f->add(new KisKSF32ColorSpaceFactory<10>);
#ifdef HAVE_OPENEXR
    f->add(new KisKSF16ColorSpaceFactory<3>);
    f->add(new KisKSF16ColorSpaceFactory<4>);
    f->add(new KisKSF16ColorSpaceFactory<6>);
    f->add(new KisKSF16ColorSpaceFactory<10>);
#endif
}

KSColorSpacesPlugin::~KSColorSpacesPlugin()
{
}

#include "ks_colorspaces.moc"
