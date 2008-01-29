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
#include "kis_kslc_colorspace.h"
#include "kis_ksqp_colorspace.h"

#include <KGenericFactory>
#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>

#ifdef HAVE_OPENEXR
#include "half.h"
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
        f->addProfile(new KisIlluminantProfile(curr));

    f->add(new KisKSLCColorSpaceFactory<float,6>);
    f->add(new KisKSLCColorSpaceFactory<float,9>);
    f->add(new KisKSQPColorSpaceFactory<float,6>);
    f->add(new KisKSQPColorSpaceFactory<float,9>);
#ifdef HAVE_OPENEXR
    f->add(new KisKSLCColorSpaceFactory<half,6>);
    f->add(new KisKSLCColorSpaceFactory<half,9>);
    f->add(new KisKSQPColorSpaceFactory<half,6>);
    f->add(new KisKSQPColorSpaceFactory<half,9>);
#endif

    QVector<const KoColorSpace *> css;
    css.append(f->colorSpace(KisKSLCColorSpace<float,6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSLCColorSpace<float,9>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSQPColorSpace<float,6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSQPColorSpace<float,9>::ColorSpaceId().id(),0));
#ifdef HAVE_OPENEXR
    css.append(f->colorSpace(KisKSLCColorSpace<half,6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSLCColorSpace<half,9>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSQPColorSpace<half,6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSQPColorSpace<half,9>::ColorSpaceId().id(),0));
#endif

    foreach(const KoColorSpace *cs, css) {
        if(!cs)
            continue;
        KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID(cs->id()+"HISTO", i18n(QString("%1 Histogram").arg(cs->name()).toUtf8().data())), cs->clone()));
    }
}

KSColorSpacesPlugin::~KSColorSpacesPlugin()
{
}

#include "ks_colorspaces.moc"
