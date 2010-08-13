/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004,2010 Cyrille Berger <cberger@cberger.net>
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
#include "LcmsEnginePlugin.h"

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

#include "IccColorSpaceEngine.h"
#include "colorprofiles/LcmsColorProfileContainer.h"
#include "colorspaces/cmyk_u8/CmykU8ColorSpace.h"
#include "colorspaces/cmyk_u16/CmykU16ColorSpace.h"
#include "colorspaces/gray_u8/GrayU8ColorSpace.h"
#include "colorspaces/lab_u16/LabColorSpace.h"
#include "colorspaces/xyz_u16/XyzU16ColorSpace.h"
#include "colorspaces/gray_u8_no_alpha/GrayU8NoAlphaColorSpace.h"
#include "colorspaces/gray_u16_no_alpha/GrayU16NoAlphaColorSpace.h"
#include "colorspaces/rgb_u8/RgbU8ColorSpace.h"
#include "colorspaces/gray_u16/GrayU16ColorSpace.h"
#include "colorspaces/rgb_u16/RgbU16ColorSpace.h"

typedef KGenericFactory<LcmsEnginePlugin> LcmsEnginePluginFactory;
K_EXPORT_COMPONENT_FACTORY(kolcmsengine, LcmsEnginePluginFactory("koffice"))

LcmsEnginePlugin::LcmsEnginePlugin(QObject *parent, const QStringList &)
        : QObject(parent)
{
    kDebug(31000) << "Initializing the lcms engine plugin";
    //setComponentData(LcmsEnginePluginFactory::componentData());
    KGlobal::locale()->insertCatalog("kocolorspaces");



    KoColorSpaceRegistry* registry = KoColorSpaceRegistry::instance();

    // Initialise color engine
    KoColorSpaceEngineRegistry::instance()->add(new IccColorSpaceEngine);

    // prepare a list of the ICC profiles
    KGlobal::mainComponent().dirs()->addResourceType("icc_profiles", 0, "share/color/icc/");

    QStringList profileFilenames;
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.icm",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.ICM",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.ICC",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.icc",  KStandardDirs::Recursive);

    // Load the profiles
    if (!profileFilenames.empty()) {
        KoColorProfile * profile = 0;
        for (QStringList::Iterator it = profileFilenames.begin(); it != profileFilenames.end(); ++it) {
            profile = new IccColorProfile(*it);
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
    KoColorProfile *labProfile = LcmsColorProfileContainer::createFromLcmsProfile(cmsCreateLab2Profile(NULL));
    registry->addProfile(labProfile);
    registry->add(new LabColorSpaceFactory());
    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
        (KoID("LABAHISTO", i18n("L*a*b* Histogram")), LABAColorModelID.id(), Integer16BitsColorDepthID.id()));

    KoColorProfile *rgbProfile = LcmsColorProfileContainer::createFromLcmsProfile(cmsCreate_sRGBProfile());
    registry->addProfile(rgbProfile);

    registry->add(new RgbU16ColorSpaceFactory());
    registry->add(new RgbU8ColorSpaceFactory());

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
        (KoID("RGB8HISTO", i18n("RGB8 Histogram")), RGBAColorModelID.id(), Integer8BitsColorDepthID.id()));

    // Create the default profile for grayscale, probably not the best place to but that, but still better than in a grayscale plugin
    // .22 gamma grayscale or something like that. Taken from the lcms tutorial...
    cmsToneCurve* Gamma = cmsBuildGamma(0, 2.2);
    cmsHPROFILE hProfile = cmsCreateGrayProfile(cmsD50_xyY(), Gamma);
    cmsFreeToneCurve(Gamma);
    KoColorProfile *defProfile = LcmsColorProfileContainer::createFromLcmsProfile(hProfile);
    registry->addProfile(defProfile);

    // Gray without alpha 8
//    KoColorSpaceFactory* csFactory = new KoGrayColorSpaceFactory();
//    registry->add(csFactory);
//
//    KoHistogramProducerFactoryRegistry::instance()->add(
//            new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
//            (KoID("GRAYA8HISTO", i18n("GRAY/Alpha8 Histogram")), csFactory->id()));

    // Gray without alpha 16
    KoColorSpaceFactory* csFactory = new GrayU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
        (KoID("GRAYA16HISTO", i18n("GRAY/Alpha16 Histogram")), GrayColorModelID.id(), Integer16BitsColorDepthID.id()));

    // Gray Alpha 8
    csFactory = new KoGrayAU8ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
        (KoID("GRAYA8HISTO", i18n("GRAY/Alpha8 Histogram")), GrayAColorModelID.id(), Integer8BitsColorDepthID.id()));

    // Gray Alpha 16
    csFactory = new KoGrayAU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
        (KoID("GRAYA16HISTO", i18n("GRAY/Alpha16 Histogram")), GrayAColorModelID.id(), Integer16BitsColorDepthID.id()));

    // CMYK 16
    csFactory = new CmykU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
        (KoID("CMYK16HISTO", i18n("CMYK16 Histogram")), CMYKAColorModelID.id(), Integer16BitsColorDepthID.id()));

    // CMYK 8
    csFactory = new CmykU8ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
        (KoID("CMYK8HISTO", i18n("CMYK8 Histogram")), CMYKAColorModelID.id(), Integer8BitsColorDepthID.id()));

    // XYZ 16
    KoColorProfile *xyzProfile = LcmsColorProfileContainer::createFromLcmsProfile(cmsCreateXYZProfile());
    registry->addProfile(xyzProfile);

    csFactory = new XyzU16ColorSpaceFactory();
    registry->add(csFactory);

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
        (KoID("XYZ16HISTO", i18n("XYZ16 Histogram")), XYZAColorModelID.id(), Integer16BitsColorDepthID.id()));

    // Add profile alias for default profile from lcms1
    registry->addProfileAlias("sRGB built-in - (lcms internal)", "sRGB built-in");
    registry->addProfileAlias("gray built-in - (lcms internal)", "gray built-in");
    registry->addProfileAlias("Lab identity built-in - (lcms internal)", "Lab identity built-in");
}

#include <LcmsEnginePlugin.moc>
