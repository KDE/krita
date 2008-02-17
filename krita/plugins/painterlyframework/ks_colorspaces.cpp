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
#include "kis_illuminant_profile_qp.h"

#include "kis_ks_colorspace.h"
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

typedef KGenericFactory<KSColorSpacesPlugin> KSColorSpacesPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritakscolorspacesplugin, KSColorSpacesPluginFactory("krita") )

KSColorSpacesPlugin::KSColorSpacesPlugin(QObject *parent, const QStringList &)
: QObject(parent)
{
    QStringList list;
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();

    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.ill",  KStandardDirs::Recursive);

    foreach(QString curr, list)
        f->addProfile(new KisIlluminantProfileQP(curr));

    f->add(new KisKSF32ColorSpaceFactory<6>);
    f->add(new KisKSF32ColorSpaceFactory<9>);
#ifdef HAVE_OPENEXR
    f->add(new KisKSF16ColorSpaceFactory<6>);
    f->add(new KisKSF16ColorSpaceFactory<9>);
#endif

    QVector<const KoColorSpace *> css;
    css.append(f->colorSpace(KisKSF32ColorSpace<6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSF32ColorSpace<9>::ColorSpaceId().id(),0));
#ifdef HAVE_OPENEXR
    css.append(f->colorSpace(KisKSF16ColorSpace<6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSF16ColorSpace<9>::ColorSpaceId().id(),0));
#endif

    foreach(const KoColorSpace *cs, css) {
        if(!cs)
            continue;
        KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID(cs->id()+"HISTO", i18n("%1 Histogram", cs->name())), cs->clone()));
    }
}

KSColorSpacesPlugin::~KSColorSpacesPlugin()
{
}

#include "ks_colorspaces.moc"
