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
#include "kis_kslinear_colorspace.h"
#include "kis_ksqp_colorspace.h"

#include <KGenericFactory>
#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>

typedef KGenericFactory<KSColorSpacesPlugin> KSColorSpacesPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritakscolorspacesplugin, KSColorSpacesPluginFactory("krita") )

KSColorSpacesPlugin::KSColorSpacesPlugin(QObject *parent, const QStringList &)
: QObject(parent)
{
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();

    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    QStringList d659 = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_9*.ill",  KStandardDirs::Recursive);

    KisIlluminantProfile *D65_9;
    foreach(QString d65, d659) {
        D65_9 = new KisIlluminantProfile(d65);
        f->addProfile(D65_9);
    }

    {
        KoColorSpace *colorSpaceKSQP = new KisKSQPColorSpace(D65_9->clone());
        KoColorSpaceFactory *csf  = new KisKSQPColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceKSQP);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
        (KoID("KS9QPF32HISTO", i18n("9-pairs KS QP Histogram")), colorSpaceKSQP) );
    }

}

KSColorSpacesPlugin::~KSColorSpacesPlugin()
{
}

#include "ks_colorspaces.moc"
