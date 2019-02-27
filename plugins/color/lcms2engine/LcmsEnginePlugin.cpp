/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include <kpluginfactory.h>
#include <KoResourcePaths.h>
#include <klocalizedstring.h>
#include <QDebug>
#include <QApplication>

#include "kis_assert.h"

#include <KoBasicHistogramProducers.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>

#include "IccColorSpaceEngine.h"
#include "colorprofiles/LcmsColorProfileContainer.h"

#include "colorspaces/cmyk_u8/CmykU8ColorSpace.h"
#include "colorspaces/cmyk_u16/CmykU16ColorSpace.h"
#include "colorspaces/cmyk_f32/CmykF32ColorSpace.h"

#include "colorspaces/gray_u8/GrayU8ColorSpace.h"
#include "colorspaces/gray_u16/GrayU16ColorSpace.h"
#include "colorspaces/gray_f32/GrayF32ColorSpace.h"

#include "colorspaces/lab_u8/LabU8ColorSpace.h"
#include "colorspaces/lab_u16/LabColorSpace.h"
#include "colorspaces/lab_f32/LabF32ColorSpace.h"

#include "colorspaces/xyz_u8/XyzU8ColorSpace.h"
#include "colorspaces/xyz_u16/XyzU16ColorSpace.h"
#include "colorspaces/xyz_f32/XyzF32ColorSpace.h"

#include "colorspaces/rgb_u8/RgbU8ColorSpace.h"
#include "colorspaces/rgb_u16/RgbU16ColorSpace.h"
#include "colorspaces/rgb_f32/RgbF32ColorSpace.h"

#include "colorspaces/ycbcr_u8/YCbCrU8ColorSpace.h"
#include "colorspaces/ycbcr_u16/YCbCrU16ColorSpace.h"
#include "colorspaces/ycbcr_f32/YCbCrF32ColorSpace.h"

#include "LcmsRGBP2020PQColorSpace.h"

#include <KoConfig.h>

#ifdef HAVE_OPENEXR
#   include <half.h>
#   ifdef HAVE_LCMS24
#       include "colorspaces/gray_f16/GrayF16ColorSpace.h"
#       include "colorspaces/xyz_f16/XyzF16ColorSpace.h"
#       include "colorspaces/rgb_f16/RgbF16ColorSpace.h"
#   endif
#endif

void lcms2LogErrorHandlerFunction(cmsContext /*ContextID*/, cmsUInt32Number ErrorCode, const char *Text)
{
    qCritical() << "Lcms2 error: " << ErrorCode << Text;
}

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "kolcmsengine.json", registerPlugin<LcmsEnginePlugin>();)

LcmsEnginePlugin::LcmsEnginePlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoResourcePaths::addResourceType("icc_profiles", "data", "/color/icc");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/profiles/");

    // Set the lmcs error reporting function
    cmsSetLogErrorHandler(&lcms2LogErrorHandlerFunction);

    KoColorSpaceRegistry *registry = KoColorSpaceRegistry::instance();

    // Initialise color engine
    KoColorSpaceEngineRegistry::instance()->add(new IccColorSpaceEngine);


    QStringList profileFilenames;
    profileFilenames += KoResourcePaths::findAllResources("icc_profiles", "*.icm",  KoResourcePaths::Recursive);
    profileFilenames += KoResourcePaths::findAllResources("icc_profiles", "*.ICM",  KoResourcePaths::Recursive);
    profileFilenames += KoResourcePaths::findAllResources("icc_profiles", "*.ICC",  KoResourcePaths::Recursive);
    profileFilenames += KoResourcePaths::findAllResources("icc_profiles", "*.icc",  KoResourcePaths::Recursive);

    QStringList iccProfileDirs;

#ifdef Q_OS_MAC
    iccProfileDirs.append(QDir::homePath() + "/Library/ColorSync/Profiles/");
    iccProfileDirs.append("/System/Library/ColorSync/Profiles/");
    iccProfileDirs.append("/Library/ColorSync/Profiles/");
#endif
#ifdef Q_OS_WIN
    QString winPath = QString::fromUtf8(qgetenv("windir"));
    winPath.replace('\\','/');
    iccProfileDirs.append(winPath + "/System32/Spool/Drivers/Color/");

#endif
    Q_FOREACH(const QString &iccProfiledir, iccProfileDirs) {
        QDir profileDir(iccProfiledir);
        Q_FOREACH(const QString &entry, profileDir.entryList(QStringList() << "*.icm" << "*.icc", QDir::NoDotAndDotDot | QDir::Files | QDir::Readable)) {
            profileFilenames << iccProfiledir + "/" + entry;
        }
    }
    // Load the profiles
    if (!profileFilenames.empty()) {
        for (QStringList::Iterator it = profileFilenames.begin(); it != profileFilenames.end(); ++it) {
            KoColorProfile *profile = new IccColorProfile(*it);
            Q_CHECK_PTR(profile);

            profile->load();
            if (profile->valid()) {
                //qDebug() << "Valid profile : " << profile->fileName() << profile->name();
                registry->addProfileToMap(profile);
            } else {
                qDebug() << "Invalid profile : " << profile->fileName() << profile->name();
                delete profile;
            }
        }
    }

    // ------------------- LAB ---------------------------------

    KoColorProfile *labProfile = LcmsColorProfileContainer::createFromLcmsProfile(cmsCreateLab2Profile(0));
    registry->addProfile(labProfile);

    registry->add(new LabU8ColorSpaceFactory());
    registry->add(new LabU16ColorSpaceFactory());
    registry->add(new LabF32ColorSpaceFactory());

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("LABAU8HISTO", i18n("L*a*b*/8 Histogram")), LABAColorModelID.id(), Integer8BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("LABAU16HISTO", i18n("L*a*b*/16 Histogram")), LABAColorModelID.id(), Integer16BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("LABAF32HISTO", i18n("L*a*b*/32 Histogram")), LABAColorModelID.id(), Float32BitsColorDepthID.id()));

    // ------------------- RGB ---------------------------------

    KoColorProfile *rgbProfile = LcmsColorProfileContainer::createFromLcmsProfile(cmsCreate_sRGBProfile());
    registry->addProfile(rgbProfile);

    registry->add(new LcmsRGBP2020PQColorSpaceFactoryWrapper<RgbU8ColorSpaceFactory>());
    registry->add(new LcmsRGBP2020PQColorSpaceFactoryWrapper<RgbU16ColorSpaceFactory>());
#ifdef HAVE_LCMS24
#ifdef HAVE_OPENEXR
    registry->add(new LcmsRGBP2020PQColorSpaceFactoryWrapper<RgbF16ColorSpaceFactory>());
#endif
#endif
    registry->add(new LcmsRGBP2020PQColorSpaceFactoryWrapper<RgbF32ColorSpaceFactory>());

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("RGBU8HISTO", i18n("RGBA/8 Histogram")), RGBAColorModelID.id(), Integer8BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("RGBU16HISTO", i18n("RGBA/16 Histogram")), RGBAColorModelID.id(), Integer16BitsColorDepthID.id()));

#ifdef HAVE_LCMS24
#ifdef HAVE_OPENEXR
    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF16HalfHistogramProducer>
            (KoID("RGBF16HISTO", i18n("RGBA/F16 Histogram")), RGBAColorModelID.id(), Float16BitsColorDepthID.id()));
#endif
#endif

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("RGF328HISTO", i18n("RGBA/F32 Histogram")), RGBAColorModelID.id(), Float32BitsColorDepthID.id()));

    // ------------------- GRAY ---------------------------------

    cmsToneCurve *Gamma = cmsBuildGamma(0, 2.2);
    cmsHPROFILE hProfile = cmsCreateGrayProfile(cmsD50_xyY(), Gamma);
    cmsFreeToneCurve(Gamma);
    KoColorProfile *defProfile = LcmsColorProfileContainer::createFromLcmsProfile(hProfile);
    registry->addProfile(defProfile);

    registry->add(new GrayAU8ColorSpaceFactory());
    registry->add(new GrayAU16ColorSpaceFactory());
#ifdef HAVE_LCMS24
#ifdef HAVE_OPENEXR
    registry->add(new GrayF16ColorSpaceFactory());
#endif
#endif
    registry->add(new GrayF32ColorSpaceFactory());

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("GRAYA8HISTO", i18n("GRAY/8 Histogram")), GrayAColorModelID.id(), Integer8BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("GRAYA16HISTO", i18n("GRAY/16 Histogram")), GrayAColorModelID.id(), Integer16BitsColorDepthID.id()));
#ifdef HAVE_LCMS24
#ifdef HAVE_OPENEXR
    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF16HalfHistogramProducer>
            (KoID("GRAYF16HISTO", i18n("GRAYF/F16 Histogram")), GrayAColorModelID.id(), Float16BitsColorDepthID.id()));
#endif
#endif

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("GRAYAF32HISTO", i18n("GRAY/F32 float Histogram")), GrayAColorModelID.id(), Float32BitsColorDepthID.id()));

    // ------------------- CMYK ---------------------------------

    registry->add(new CmykU8ColorSpaceFactory());
    registry->add(new CmykU16ColorSpaceFactory());
    registry->add(new CmykF32ColorSpaceFactory());

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("CMYK8HISTO", i18n("CMYK/8 Histogram")), CMYKAColorModelID.id(), Integer8BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("CMYK16HISTO", i18n("CMYK/16 Histogram")), CMYKAColorModelID.id(), Integer16BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("CMYKF32HISTO", i18n("CMYK/F32 Histogram")), CMYKAColorModelID.id(), Float32BitsColorDepthID.id()));

    // ------------------- XYZ ---------------------------------

    KoColorProfile *xyzProfile = LcmsColorProfileContainer::createFromLcmsProfile(cmsCreateXYZProfile());
    registry->addProfile(xyzProfile);

    registry->add(new XyzU8ColorSpaceFactory());
    registry->add(new XyzU16ColorSpaceFactory());
#ifdef HAVE_LCMS24
#ifdef HAVE_OPENEXR
    registry->add(new XyzF16ColorSpaceFactory());
#endif
#endif
    registry->add(new XyzF32ColorSpaceFactory());

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("XYZ8HISTO", i18n("XYZ/8 Histogram")), XYZAColorModelID.id(), Integer8BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("XYZ16HISTO", i18n("XYZ/16 Histogram")), XYZAColorModelID.id(), Integer16BitsColorDepthID.id()));

#ifdef HAVE_LCMS24
#ifdef HAVE_OPENEXR
    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("XYZF16HISTO", i18n("XYZ/F16 Histogram")), XYZAColorModelID.id(), Float16BitsColorDepthID.id()));
#endif
#endif

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("XYZF32HISTO", i18n("XYZF32 Histogram")), XYZAColorModelID.id(), Float32BitsColorDepthID.id()));

    // ------------------- YCBCR ---------------------------------

    //    KoColorProfile *yCbCrProfile = LcmsColorProfileContainer::createFromLcmsProfile(cmsCreateYCBCRProfile());
    //    registry->addProfile(yCbCrProfile);

    registry->add(new YCbCrU8ColorSpaceFactory());
    registry->add(new YCbCrU16ColorSpaceFactory());
    registry->add(new YCbCrF32ColorSpaceFactory());

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("YCBCR8HISTO", i18n("YCbCr/8 Histogram")), YCbCrAColorModelID.id(), Integer8BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
            (KoID("YCBCR16HISTO", i18n("YCbCr/16 Histogram")), YCbCrAColorModelID.id(), Integer16BitsColorDepthID.id()));

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicF32HistogramProducer>
            (KoID("YCBCRF32HISTO", i18n("YCbCr/F32 Histogram")), YCbCrAColorModelID.id(), Float32BitsColorDepthID.id()));

    // Add profile alias for default profile from lcms1
    registry->addProfileAlias("sRGB built-in - (lcms internal)", "sRGB built-in");
    registry->addProfileAlias("gray built-in - (lcms internal)", "gray built-in");
    registry->addProfileAlias("Lab identity built-in - (lcms internal)", "Lab identity built-in");
    registry->addProfileAlias("XYZ built-in - (lcms internal)", "XYZ identity built-in");
}

#include <LcmsEnginePlugin.moc>
