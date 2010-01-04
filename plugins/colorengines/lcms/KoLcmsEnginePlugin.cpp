/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "KoLcmsEnginePlugin.h"

#include <QHash>

#include <QStringList>
#include <QDir>

#include <kcomponentdata.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kglobal.h>

#include <KoBasicHistogramProducers.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoColorProfileFactory.h>

#include "KoIccColorSpaceEngine.h"
#include "colorprofiles/KoLcmsColorProfileContainer.h"
#include "colorspaces/cmyk_u8/KoCmykU8ColorSpace.h"
#include "colorspaces/cmyk_u16/KoCmykU16ColorSpace.h"
#include "colorspaces/gray_u8/KoGrayU8ColorSpace.h"
#include "colorspaces/lab_u16/KoLabColorSpace.h"
#include "colorspaces/xyz_u16/KoXyzU16ColorSpace.h"
#include "colorspaces/gray_u8_no_alpha/KoGrayU8NoAlphaColorSpace.h"
#include "colorspaces/gray_u16_no_alpha/KoGrayU16NoAlphaColorSpace.h"
#include "colorspaces/rgb_u8/KoRgbU8ColorSpace.h"
#include "colorspaces/gray_u16/KoGrayU16ColorSpace.h"
#include "colorspaces/rgb_u16/KoRgbU16ColorSpace.h"

class KoIccColorProfileFactory : public KoColorProfileFactory {

public:

    KoColorProfile* createColorProfile(const QByteArray& rawData) {
        return new KoIccColorProfile(rawData);
    }

};

typedef KGenericFactory<KoLcmsEnginePlugin> KoLcmsEnginePluginFactory;
K_EXPORT_COMPONENT_FACTORY(kolcmsengine, KoLcmsEnginePluginFactory("koffice"))

        KoLcmsEnginePlugin::KoLcmsEnginePlugin(QObject *parent, const QStringList &)
            : QObject(parent)
{
    kDebug(31000) << "Initializing the lcms engine plugin";
    //setComponentData(KoLcmsEnginePluginFactory::componentData());
    KGlobal::locale()->insertCatalog("kocolorspaces");



    KoColorSpaceRegistry* registry = KoColorSpaceRegistry::instance();
    registry->addColorProfileFactory("icc", new KoIccColorProfileFactory());

    // Initialise color engine
    KoColorSpaceEngineRegistry::instance()->add(new KoIccColorSpaceEngine);

    // prepare a list of the ICC profiles
    KGlobal::mainComponent().dirs()->addResourceType("icc_profiles", 0, "share/color/icc/");

    QStringList profileFilenames;
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.icm",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.ICM",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.ICC",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.icc",  KStandardDirs::Recursive);

    // Set lcms to return NUll/false etc from failing calls, rather than aborting the app.
    cmsErrorAction(LCMS_ERROR_SHOW);

    // Load the profiles
    if (!profileFilenames.empty()) {
        KoColorProfile * profile = 0;
        for (QStringList::Iterator it = profileFilenames.begin(); it != profileFilenames.end(); ++it) {
            profile = new KoIccColorProfile(*it);
            Q_CHECK_PTR(profile);

            profile->load();
            if (profile->valid()) {
                kDebug(31000) << "Valid profile : " << profile->fileName() << profile->name();
                registry->addProfileToMap(profile);
            } else {
                kDebug(31000) << "Invalid profile : " << profile->fileName() << profile->name();
                delete profile;
            }
        }
    }

    // Initialise LAB
    KoColorProfile *labProfile = KoLcmsColorProfileContainer::createFromLcmsProfile(cmsCreateLabProfile(NULL));
    registry->addProfile(labProfile);
    registry->add(new KoLabColorSpaceFactory());
    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("LABAHISTO", i18n("L*a*b* Histogram")), KoLabColorSpace::colorSpaceId()));

    KoColorProfile *rgbProfile = KoLcmsColorProfileContainer::createFromLcmsProfile(cmsCreate_sRGBProfile());
    registry->addProfile(rgbProfile);

    registry->add(new KoRgbU16ColorSpaceFactory());
    registry->add(new KoRgbU8ColorSpaceFactory());

    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("RGB8HISTO", i18n("RGB8 Histogram")), KoRgbU8ColorSpace::colorSpaceId()));

    // Create the default profile for grayscale, probably not the best place to but that, but still better than in a grayscale plugin
    // .22 gamma grayscale or something like that. Taken from the lcms tutorial...
    LPGAMMATABLE Gamma = cmsBuildGamma(256, 2.2);
    cmsHPROFILE hProfile = cmsCreateGrayProfile(cmsD50_xyY(), Gamma);
    cmsFreeGamma(Gamma);
    KoColorProfile *defProfile = KoLcmsColorProfileContainer::createFromLcmsProfile(hProfile);
    registry->addProfile(defProfile);

    // Gray without alpha 8
//    KoColorSpaceFactory* csFactory = new KoGrayColorSpaceFactory();
//    registry->add(csFactory);
//
//    KoHistogramProducerFactoryRegistry::instance()->add(
//            new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
//            (KoID("GRAYA8HISTO", i18n("GRAY/Alpha8 Histogram")), csFactory->id()));

    // Gray without alpha 16
    KoColorSpaceFactory* csFactory = new KoGrayU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("GRAYA16HISTO", i18n("GRAY/Alpha16 Histogram")), csFactory->id()));

    // Gray Alpha 8
    csFactory = new KoGrayAU8ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("GRAYA8HISTO", i18n("GRAY/Alpha8 Histogram")), csFactory->id()) );

    // Gray Alpha 16
    csFactory = new KoGrayAU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("GRAYA16HISTO", i18n("GRAY/Alpha16 Histogram")), csFactory->id()) );

    // CMYK 16
    csFactory = new KoCmykU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("CMYK16HISTO", i18n("CMYK16 Histogram")), csFactory->id()) );

    // CMYK 8
    csFactory = new KoCmykU8ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("CMYK8HISTO", i18n("CMYK8 Histogram")), csFactory->id()) );

    // XYZ 16
    KoColorProfile *xyzProfile = KoLcmsColorProfileContainer::createFromLcmsProfile(cmsCreateXYZProfile());
    registry->addProfile(xyzProfile);

    csFactory = new KoXyzU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("XYZ16HISTO", i18n("XYZ16 Histogram")), csFactory->id()));
}

#include "KoLcmsEnginePlugin.moc"
