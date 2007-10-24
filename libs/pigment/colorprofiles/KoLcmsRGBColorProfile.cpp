#if 0
/*
 * Copyright (C) 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoLcmsRGBColorProfile.h"

#include <kdebug.h>

class KoLcmsRGBColorProfile::Private
{
public:
    Private()
    {
    }

    Chromaticities chromaticities;
};

KoLcmsRGBColorProfile::KoLcmsRGBColorProfile(const Chromaticities &chromaticities, double gamma, const QString &profileName)
    : d(new Private())
{
    const int numGammaTableEntries = 256;
    LPGAMMATABLE gammaTable = cmsBuildGamma(numGammaTableEntries, gamma);

    const int numTransferFunctions = 3;
    LPGAMMATABLE transferFunctions[numTransferFunctions];

    for (int i = 0; i < numTransferFunctions; ++i) {
        transferFunctions[i] = gammaTable;
    }

    cmsHPROFILE profile = cmsCreateRGBProfile(const_cast<cmsCIExyY *>(&chromaticities.whitePoint), 
                                              const_cast<cmsCIExyYTRIPLE *>(&chromaticities.primaries), 
                                              transferFunctions);
    QString name = profileName;

    if (name.isEmpty()) {
        name = QString("lcms virtual RGB profile - R(%1, %2) G(%3, %4) B(%5, %6) W(%7, %8) gamma %9")
                                   .arg(chromaticities.primaries.Red.x)
                                   .arg(chromaticities.primaries.Red.y)
                                   .arg(chromaticities.primaries.Green.x)
                                   .arg(chromaticities.primaries.Green.y)
                                   .arg(chromaticities.primaries.Blue.x)
                                   .arg(chromaticities.primaries.Blue.y)
                                   .arg(chromaticities.whitePoint.x)
                                   .arg(chromaticities.whitePoint.y)
                                   .arg(gamma);
    }

    // icSigProfileDescriptionTag is the compulsory tag and is the profile name
    // displayed by other applications.
    cmsAddTag(profile, icSigProfileDescriptionTag, name.toLatin1().data());

    cmsAddTag(profile, icSigDeviceModelDescTag, name.toLatin1().data());

    // Clear the default manufacturer's tag that is set to "(lcms internal)" 
    QByteArray ba("");
    cmsAddTag(profile, icSigDeviceMfgDescTag, ba.data());

    cmsFreeGamma(gammaTable);

    setProfile(profile);

    d->chromaticities = chromaticities;
}

KoLcmsRGBColorProfile::~KoLcmsRGBColorProfile()
{
    delete d;
}

KoLcmsRGBColorProfile::Chromaticities KoLcmsRGBColorProfile::chromaticities() const
{
    return d->chromaticities;
}

static cmsCIExyY RGB2xyY(cmsHPROFILE RGBProfile, double red, double green, double blue)
{
    cmsHPROFILE XYZProfile = cmsCreateXYZProfile();

    const DWORD inputFormat = TYPE_RGB_DBL;
    const DWORD outputFormat = TYPE_XYZ_DBL;
    const DWORD transformFlags = cmsFLAGS_NOTPRECALC;

    cmsHTRANSFORM transform = cmsCreateTransform(RGBProfile, inputFormat, XYZProfile, outputFormat, 
                                                 INTENT_ABSOLUTE_COLORIMETRIC, transformFlags);

    struct XYZPixel {
        double X;
        double Y;
        double Z;
    };
    struct RGBPixel {
        double red;
        double green;
        double blue;
    };

    XYZPixel xyzPixel;
    RGBPixel rgbPixel;

    rgbPixel.red = red;
    rgbPixel.green = green;
    rgbPixel.blue = blue;

    const unsigned int numPixelsToTransform = 1;

    cmsDoTransform(transform, &rgbPixel, &xyzPixel, numPixelsToTransform);

    cmsCIEXYZ xyzPixelXYZ;

    xyzPixelXYZ.X = xyzPixel.X;
    xyzPixelXYZ.Y = xyzPixel.Y;
    xyzPixelXYZ.Z = xyzPixel.Z;

    cmsCIExyY xyzPixelxyY;

    cmsXYZ2xyY(&xyzPixelxyY, &xyzPixelXYZ);

    cmsDeleteTransform(transform);
    cmsCloseProfile(XYZProfile);

    return xyzPixelxyY;
}

KoLcmsRGBColorProfile::Chromaticities KoLcmsRGBColorProfile::chromaticitiesFromProfile(cmsHPROFILE profile)
{
    Q_ASSERT(cmsGetColorSpace(profile) == icSigRgbData);

    Chromaticities chromaticities;

    chromaticities.primaries.Red = RGB2xyY(profile, 1.0f, 0.0f, 0.0f);
    chromaticities.primaries.Green = RGB2xyY(profile, 0.0f, 1.0f, 0.0f);
    chromaticities.primaries.Blue = RGB2xyY(profile, 0.0f, 0.0f, 1.0f);
    chromaticities.whitePoint = RGB2xyY(profile, 1.0f, 1.0f, 1.0f);

    return chromaticities;
}

#endif
