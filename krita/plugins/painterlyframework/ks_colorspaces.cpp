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
    QString curr;
    KisIlluminantProfile *ill;

    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.ill",  KStandardDirs::Recursive);

    foreach(curr, list) {
        ill = new KisIlluminantProfile(curr);
        f->addProfile(ill);
    }

    curr = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_6_high.ill",  KStandardDirs::Recursive)[0];
    ill  = new KisIlluminantProfile(curr);
    {
        KoColorSpace *colorSpaceKSQP = new KisKSQPColorSpace<float,6>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSQPColorSpaceFactory<float,6>();
        Q_CHECK_PTR(colorSpaceKSQP);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSQP6F32HISTO", i18n("6-pairs KS QP F32 Histogram")), colorSpaceKSQP) );
    }
#ifdef HAVE_OPENEXR
    {
        KoColorSpace *colorSpaceKSQP = new KisKSQPColorSpace<half,6>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSQPColorSpaceFactory<half,6>();
        Q_CHECK_PTR(colorSpaceKSQP);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSQP6F16HISTO", i18n("6-pairs KS QP Half Histogram")), colorSpaceKSQP) );
    }
#endif
    delete ill;

    curr = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_6_high.ill",  KStandardDirs::Recursive)[0];
    ill  = new KisIlluminantProfile(curr);
    {
        KoColorSpace *colorSpaceKS = new KisKSLCColorSpace<float,6>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSLCColorSpaceFactory<float,6>();
        Q_CHECK_PTR(colorSpaceKS);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSLC6F32HISTO", i18n("6-pairs KS LC F32 Histogram")), colorSpaceKS) );
    }
#ifdef HAVE_OPENEXR
    {
        KoColorSpace *colorSpaceKS = new KisKSLCColorSpace<half,6>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSLCColorSpaceFactory<half,6>();
        Q_CHECK_PTR(colorSpaceKS);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSLC6F16HISTO", i18n("6-pairs KS LC Half Histogram")), colorSpaceKS) );
    }
#endif
    delete ill;

    curr = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_9_high.ill",  KStandardDirs::Recursive)[0];
    ill  = new KisIlluminantProfile(curr);
    {
        KoColorSpace *colorSpaceKS = new KisKSLCColorSpace<float,9>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSLCColorSpaceFactory<float,9>();
        Q_CHECK_PTR(colorSpaceKS);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSLC9F32HISTO", i18n("9-pairs KS LC F32 Histogram")), colorSpaceKS) );
    }
#ifdef HAVE_OPENEXR
    {
        KoColorSpace *colorSpaceKS = new KisKSLCColorSpace<half,9>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSLCColorSpaceFactory<half,9>();
        Q_CHECK_PTR(colorSpaceKS);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSLC9F16HISTO", i18n("9-pairs KS LC Half Histogram")), colorSpaceKS) );
    }
#endif
    delete ill;
/*
    curr = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_12_high.ill",  KStandardDirs::Recursive)[0];
    ill  = new KisIlluminantProfile(curr);
    {
        KoColorSpace *colorSpaceKS = new KisKSLCColorSpace<float,12>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSLCColorSpaceFactory<float,12>();
        Q_CHECK_PTR(colorSpaceKS);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSLC12F32HISTO", i18n("12-pairs KS LC F32 Histogram")), colorSpaceKS) );
    }
    #ifdef HAVE_OPENEXR
    {
        KoColorSpace *colorSpaceKS = new KisKSLCColorSpace<half,12>(ill->clone());
        KoColorSpaceFactory *csf  = new KisKSLCColorSpaceFactory<half,12>();
        Q_CHECK_PTR(colorSpaceKS);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KSLC12F16HISTO", i18n("12-pairs KS LC Half Histogram")), colorSpaceKS) );
    }
    #endif
    delete ill;
*/
}

KSColorSpacesPlugin::~KSColorSpacesPlugin()
{
}

#include "ks_colorspaces.moc"
