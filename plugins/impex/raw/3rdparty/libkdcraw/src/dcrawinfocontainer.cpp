/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2007-05-02
 * @brief  RAW file identification information container
 *
 * @author Copyright (C) 2007-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Local includes

#include "dcrawinfocontainer.h"

namespace KDcrawIface
{

DcrawInfoContainer::DcrawInfoContainer()
{
    sensitivity       = -1.0;
    exposureTime      = -1.0;
    aperture          = -1.0;
    focalLength       = -1.0;
    pixelAspectRatio  = 1.0;    // Default value. This can be unavailable (depending of camera model).
    rawColors         = -1;
    rawImages         = -1;
    hasIccProfile     = false;
    isDecodable       = false;
    daylightMult[0]   = 0.0;
    daylightMult[1]   = 0.0;
    daylightMult[2]   = 0.0;
    cameraMult[0]     = 0.0;
    cameraMult[1]     = 0.0;
    cameraMult[2]     = 0.0;
    cameraMult[3]     = 0.0;
    blackPoint        = 0;

    for (int ch=0; ch<4; ch++)
    {
        blackPointCh[ch] = 0;
    }

    whitePoint        = 0;
    topMargin         = 0;
    leftMargin        = 0;
    orientation       = ORIENTATION_NONE;

    for (int x=0 ; x<3 ; x++)
    {
        for (int y=0 ; y<4 ; y++)
        {
            cameraColorMatrix1[x][y] = 0.0;
            cameraColorMatrix2[x][y] = 0.0;
            cameraXYZMatrix[y][x]    = 0.0;       // NOTE: see B.K.O # 253911 : [y][x] not [x][y]
        }
    }
}

DcrawInfoContainer::~DcrawInfoContainer()
{
}

bool DcrawInfoContainer::isEmpty()
{
    if (make.isEmpty()                  &&
        model.isEmpty()                 &&
        filterPattern.isEmpty()         &&
        colorKeys.isEmpty()             &&
        DNGVersion.isEmpty()            &&
        exposureTime     == -1.0        &&
        aperture         == -1.0        &&
        focalLength      == -1.0        &&
        pixelAspectRatio == 1.0         &&
        sensitivity      == -1.0        &&
        rawColors        == -1          &&
        rawImages        == -1          &&
        blackPoint       == 0           &&
        blackPointCh[0]  == 0           &&
        blackPointCh[1]  == 0           &&
        blackPointCh[2]  == 0           &&
        blackPointCh[3]  == 0           &&
        whitePoint       == 0           &&
        topMargin        == 0           &&
        leftMargin       == 0           &&
        !dateTime.isValid()             &&
        !imageSize.isValid()            &&
        !fullSize.isValid()             &&
        !outputSize.isValid()           &&
        !thumbSize.isValid()            &&
        cameraColorMatrix1[0][0] == 0.0 &&
        cameraColorMatrix1[0][1] == 0.0 &&
        cameraColorMatrix1[0][2] == 0.0 &&
        cameraColorMatrix1[0][3] == 0.0 &&
        cameraColorMatrix1[1][0] == 0.0 &&
        cameraColorMatrix1[1][1] == 0.0 &&
        cameraColorMatrix1[1][2] == 0.0 &&
        cameraColorMatrix1[1][3] == 0.0 &&
        cameraColorMatrix1[2][0] == 0.0 &&
        cameraColorMatrix1[2][1] == 0.0 &&
        cameraColorMatrix1[2][2] == 0.0 &&
        cameraColorMatrix1[2][3] == 0.0 &&
        cameraColorMatrix2[0][0] == 0.0 &&
        cameraColorMatrix2[0][1] == 0.0 &&
        cameraColorMatrix2[0][2] == 0.0 &&
        cameraColorMatrix2[0][3] == 0.0 &&
        cameraColorMatrix2[1][0] == 0.0 &&
        cameraColorMatrix2[1][1] == 0.0 &&
        cameraColorMatrix2[1][2] == 0.0 &&
        cameraColorMatrix2[1][3] == 0.0 &&
        cameraColorMatrix2[2][0] == 0.0 &&
        cameraColorMatrix2[2][1] == 0.0 &&
        cameraColorMatrix2[2][2] == 0.0 &&
        cameraColorMatrix2[2][3] == 0.0 &&
        cameraXYZMatrix[0][0]    == 0.0 &&
        cameraXYZMatrix[0][1]    == 0.0 &&
        cameraXYZMatrix[0][2]    == 0.0 &&
        cameraXYZMatrix[1][0]    == 0.0 &&
        cameraXYZMatrix[1][1]    == 0.0 &&
        cameraXYZMatrix[1][2]    == 0.0 &&
        cameraXYZMatrix[2][0]    == 0.0 &&
        cameraXYZMatrix[2][1]    == 0.0 &&
        cameraXYZMatrix[2][2]    == 0.0 &&
        cameraXYZMatrix[3][0]    == 0.0 &&
        cameraXYZMatrix[3][1]    == 0.0 &&
        cameraXYZMatrix[3][2]    == 0.0 &&
        orientation              == ORIENTATION_NONE
       )
    {
        return true;
    }
    else
    {
        return false;
    }
}

QDebug operator<<(QDebug dbg, const DcrawInfoContainer& c)
{
    dbg.nospace() << "DcrawInfoContainer::sensitivity: "      << c.sensitivity      << ", ";
    dbg.nospace() << "DcrawInfoContainer::exposureTime: "     << c.exposureTime     << ", ";
    dbg.nospace() << "DcrawInfoContainer::aperture: "         << c.aperture         << ", ";
    dbg.nospace() << "DcrawInfoContainer::focalLength: "      << c.focalLength      << ", ";
    dbg.nospace() << "DcrawInfoContainer::pixelAspectRatio: " << c.pixelAspectRatio << ", ";
    dbg.nospace() << "DcrawInfoContainer::rawColors: "        << c.rawColors        << ", ";
    dbg.nospace() << "DcrawInfoContainer::rawImages: "        << c.rawImages        << ", ";
    dbg.nospace() << "DcrawInfoContainer::hasIccProfile: "    << c.hasIccProfile    << ", ";
    dbg.nospace() << "DcrawInfoContainer::isDecodable: "      << c.isDecodable      << ", ";
    dbg.nospace() << "DcrawInfoContainer::daylightMult: "     << c.daylightMult     << ", ";
    dbg.nospace() << "DcrawInfoContainer::cameraMult: "       << c.cameraMult       << ", ";
    dbg.nospace() << "DcrawInfoContainer::blackPoint: "       << c.blackPoint       << ", ";
    dbg.nospace() << "DcrawInfoContainer::whitePoint: "       << c.whitePoint       << ", ";
    dbg.nospace() << "DcrawInfoContainer::topMargin: "        << c.topMargin        << ", ";
    dbg.nospace() << "DcrawInfoContainer::leftMargin: "       << c.leftMargin       << ", ";
    dbg.nospace() << "DcrawInfoContainer::orientation: "      << c.orientation;
    return dbg.space();
}

} // namespace KDcrawIface
