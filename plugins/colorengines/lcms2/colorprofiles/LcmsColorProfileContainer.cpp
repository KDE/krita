/*
 * This file is part of the KDE project
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "LcmsColorProfileContainer.h"

#include <cfloat>
#include <cmath>
#include <QTransform>
#include <QGenericMatrix>

#include <QDebug>

class LcmsColorProfileContainer::Private
{
public:
    Private()
        : valid(false)
        , suitableForOutput(false) { }

    cmsHPROFILE profile;
    cmsColorSpaceSignature colorSpaceSignature;
    cmsProfileClassSignature deviceClass;
    QString productDescription;
    QString manufacturer;
    QString name;
    IccColorProfile::Data * data;
    bool valid;
    bool suitableForOutput;
    bool hasColorants;
    bool adaptedFromD50;
    cmsNAMEDCOLORLIST *namedColorList;
    cmsCIEXYZ mediaWhitePoint;
    cmsCIExyY whitePoint;
    cmsCIEXYZTRIPLE colorants;
    cmsToneCurve *redTRC;
    cmsToneCurve *greenTRC;
    cmsToneCurve *blueTRC;
    cmsToneCurve *grayTRC;
};

LcmsColorProfileContainer::LcmsColorProfileContainer()
    : d(new Private())
{
    d->profile = 0;
}

LcmsColorProfileContainer::LcmsColorProfileContainer(IccColorProfile::Data * data)
    : d(new Private())
{
    d->data = data;
    d->profile = 0;
    init();
}

QByteArray LcmsColorProfileContainer::lcmsProfileToByteArray(const cmsHPROFILE profile)
{
    cmsUInt32Number  bytesNeeded = 0;
    // Make a raw data image ready for saving
    cmsSaveProfileToMem(profile, 0, &bytesNeeded); // calc size
    QByteArray rawData;
    rawData.resize(bytesNeeded);
    if (rawData.size() >= (int)bytesNeeded) {
        cmsSaveProfileToMem(profile, rawData.data(), &bytesNeeded); // fill buffer
    } else {
        qWarning() << "Couldn't resize the profile buffer, system is probably running out of memory.";
        rawData.resize(0);
    }
    return rawData;
}

IccColorProfile* LcmsColorProfileContainer::createFromLcmsProfile(const cmsHPROFILE profile)
{
    IccColorProfile* iccprofile = new IccColorProfile(lcmsProfileToByteArray(profile));
    cmsCloseProfile(profile);
    return iccprofile;
}

LcmsColorProfileContainer::~LcmsColorProfileContainer()
{
    cmsCloseProfile(d->profile);
    delete d;
}

#define _BUFFER_SIZE_ 1000

bool LcmsColorProfileContainer::init()
{
    if (d->profile) cmsCloseProfile(d->profile);

    d->profile = cmsOpenProfileFromMem((void*)d->data->rawData().constData(), d->data->rawData().size());

#ifndef NDEBUG
    if (d->data->rawData().size() == 4096) {
        qWarning() << "Profile has a size of 4096, which is suspicious and indicates a possible misuse of QIODevice::read(int), check your code.";
    }
#endif

    if (d->profile) {
        wchar_t buffer[_BUFFER_SIZE_];
        d->colorSpaceSignature = cmsGetColorSpace(d->profile);
        d->deviceClass = cmsGetDeviceClass(d->profile);
        cmsGetProfileInfo(d->profile, cmsInfoDescription, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->name = QString::fromWCharArray(buffer);

        cmsGetProfileInfo(d->profile, cmsInfoModel, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->productDescription = QString::fromWCharArray(buffer);

        cmsGetProfileInfo(d->profile, cmsInfoManufacturer, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->manufacturer = QString::fromWCharArray(buffer);
        
        cmsProfileClassSignature profile_class;
        profile_class = cmsGetDeviceClass(d->profile);
        d->valid = (profile_class != cmsSigNamedColorClass);
        
        
        //This is where obtain the whitepoint, and convert it to the actual white point of the profile in the case a Chromatic adaption tag is
        //present. This is necessary for profiles following the v4 spec.
        cmsCIEXYZ baseMediaWhitePoint;//dummy to hold copy of mediawhitepoint if this is modified by chromatic adaption.
        if (cmsIsTag(d->profile, cmsSigMediaWhitePointTag)) {
            d->mediaWhitePoint = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigMediaWhitePointTag));
            baseMediaWhitePoint = d->mediaWhitePoint;
            cmsXYZ2xyY(&d->whitePoint, &d->mediaWhitePoint);

            if (cmsIsTag(d->profile, cmsSigChromaticAdaptationTag)) {
                //the chromatic adaption tag represent a matrix from the actual white point of the profile to D50.
                cmsCIEXYZ *CAM1 = (cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigChromaticAdaptationTag);
                //We first put all our data into structures we can manipulate.
                double d3dummy [3] = {d->mediaWhitePoint.X, d->mediaWhitePoint.Y,d->mediaWhitePoint.Z};
                QGenericMatrix<1,3,double> whitePointMatrix(d3dummy);
                QTransform invertDummy(CAM1[0].X, CAM1[0].Y,CAM1[0].Z, CAM1[1].X, CAM1[1].Y,CAM1[1].Z, CAM1[2].X, CAM1[2].Y,CAM1[2].Z);
                //we then abuse QTransform's invert function because it probably does matrix invertion 20 times better than I can program.
                //if the matrix is uninvertable, invertedDummy will be an identity matrix, which for us means that it won't give any noticeble
                //effect when we start multiplying.
                QTransform invertedDummy=invertDummy.inverted();
                //we then put the QTransform into a generic 3x3 matrix.
                double d9dummy [9] = {invertedDummy.m11(), invertedDummy.m12(), invertedDummy.m13(),
                                      invertedDummy.m21(), invertedDummy.m22(), invertedDummy.m23(),
                                      invertedDummy.m31(), invertedDummy.m32(), invertedDummy.m33()};
                QGenericMatrix<3,3,double> chromaticAdaptionMatrix(d9dummy);
                //multiplying our inverted adaption matrix with the whitepoint gives us the right whitepoint.
                QGenericMatrix<1,3,double> result = chromaticAdaptionMatrix * whitePointMatrix;
                //and then we pour the matrix into the whitepoint variable. Generic matrix does row/column for indices even though it
                //uses column/row for initialising.
                d->mediaWhitePoint.X = result(0,0);
                d->mediaWhitePoint.Y = result(1,0);
                d->mediaWhitePoint.Z = result(2,0);
                cmsXYZ2xyY(&d->whitePoint, &d->mediaWhitePoint);
            }
        }
        
        //Colorant table tag is for CMYK and other named color profiles. If we use this correctly we can display the full list of named colors
        //in a named color profile. We retrieve more information elsewhere.
        if (cmsIsTag(d->profile, cmsSigColorantTableTag)) {
            d->namedColorList = ((cmsNAMEDCOLORLIST *)cmsReadTag (d->profile, cmsSigColorantTableTag));
            for (cmsUInt16Number i=0;i<cmsNamedColorCount(d->namedColorList);i++) {
                char name;
                char prefix;
                char suffix;
                cmsUInt16Number pcs;
                cmsUInt16Number col;
                cmsNamedColorInfo(d->namedColorList, i, &name, &prefix, &suffix, &pcs, &col);
                //qDebug()<<d->name<<i<<","<< name<<","<< prefix<<","<< suffix;
                //if (pcs){qDebug()<<pcs;} else {qDebug()<<"no pcs retrieved";}
            }
        }
        //This is for RGB profiles, but it only works for matrix profiles. Need to design it to work with non-matrix profiles.
        if (cmsIsTag(d->profile, cmsSigRedColorantTag)) {
            cmsCIEXYZTRIPLE tempColorants = { -1 };
            tempColorants.Red = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigRedColorantTag));
            tempColorants.Green = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigGreenColorantTag));
            tempColorants.Blue = *((cmsCIEXYZ *)cmsReadTag (d->profile, cmsSigBlueColorantTag));
            //convert to d65, this is useless.
            cmsAdaptToIlluminant(&d->colorants.Red  , &baseMediaWhitePoint, &d->mediaWhitePoint, &tempColorants.Red);
            cmsAdaptToIlluminant(&d->colorants.Green, &baseMediaWhitePoint, &d->mediaWhitePoint, &tempColorants.Green);
            cmsAdaptToIlluminant(&d->colorants.Blue , &baseMediaWhitePoint, &d->mediaWhitePoint, &tempColorants.Blue);
            //d->colorants = tempColorants;
            d->hasColorants = true;
        } else {
        //qDebug()<<d->name<<": has no colorants";
        d->hasColorants = false;
        }
        //retrieve TRC.
        if (cmsIsTag(d->profile, cmsSigRedTRCTag) && cmsIsTag(d->profile, cmsSigBlueTRCTag) && cmsIsTag(d->profile, cmsSigGreenTRCTag)) {
        
            d->redTRC = ((cmsToneCurve *)cmsReadTag (d->profile, cmsSigRedTRCTag));
            d->greenTRC = ((cmsToneCurve *)cmsReadTag (d->profile, cmsSigGreenTRCTag));
            d->blueTRC = ((cmsToneCurve *)cmsReadTag (d->profile, cmsSigBlueTRCTag));
        
        } else if (cmsIsTag(d->profile, cmsSigGrayTRCTag)) {
            d->grayTRC = ((cmsToneCurve *)cmsReadTag (d->profile, cmsSigGrayTRCTag));
        }
        
        
        // Check if the profile can convert (something->this)
        d->suitableForOutput = cmsIsMatrixShaper(d->profile)
                || ( cmsIsCLUT(d->profile, INTENT_PERCEPTUAL, LCMS_USED_AS_INPUT) &&
                     cmsIsCLUT(d->profile, INTENT_PERCEPTUAL, LCMS_USED_AS_OUTPUT) );
        return true;
    }

    return false;
}

cmsHPROFILE LcmsColorProfileContainer::lcmsProfile() const
{
    return d->profile;
}

cmsColorSpaceSignature LcmsColorProfileContainer::colorSpaceSignature() const
{
    return d->colorSpaceSignature;
}

cmsProfileClassSignature LcmsColorProfileContainer::deviceClass() const
{
    return d->deviceClass;
}

QString LcmsColorProfileContainer::manufacturer() const
{
    return d->manufacturer;
}

bool LcmsColorProfileContainer::valid() const
{
    return d->valid;
}

bool LcmsColorProfileContainer::isSuitableForOutput() const
{
    return d->suitableForOutput;
}

bool LcmsColorProfileContainer::isSuitableForPrinting() const
{
    return deviceClass() == cmsSigOutputClass;
}

bool LcmsColorProfileContainer::isSuitableForDisplay() const
{
    return deviceClass() == cmsSigDisplayClass;
}
bool LcmsColorProfileContainer::hasColorants() const
{
    return d->hasColorants;
}
QVector <double> LcmsColorProfileContainer::getColorantsXYZ() const
{
    QVector <double> colorants(9);
    colorants[0] = d->colorants.Red.X;
    colorants[1] = d->colorants.Red.Y;
    colorants[2] = d->colorants.Red.Z;
    colorants[3] = d->colorants.Green.X;
    colorants[4] = d->colorants.Green.Y;
    colorants[5] = d->colorants.Green.Z;
    colorants[6] = d->colorants.Blue.X;
    colorants[7] = d->colorants.Blue.Y;
    colorants[8] = d->colorants.Blue.Z;
    return colorants;
}

QVector <double> LcmsColorProfileContainer::getColorantsxyY() const
{
    cmsCIEXYZ temp1;
    cmsCIExyY temp2;
    QVector <double> colorants(9);
    
    temp1.X = d->colorants.Red.X;
    temp1.Y = d->colorants.Red.Y;
    temp1.Z = d->colorants.Red.Z;
    cmsXYZ2xyY(&temp2, &temp1);
    colorants[0] = temp2.x;
    colorants[1] = temp2.y;
    colorants[2] = temp2.Y;
    
    temp1.X = d->colorants.Green.X;
    temp1.Y = d->colorants.Green.Y;
    temp1.Z = d->colorants.Green.Z;
    cmsXYZ2xyY(&temp2, &temp1);
    colorants[3] = temp2.x;
    colorants[4] = temp2.y;
    colorants[5] = temp2.Y;
    
    temp1.X = d->colorants.Blue.X;
    temp1.Y = d->colorants.Blue.Y;
    temp1.Z = d->colorants.Blue.Z;
    cmsXYZ2xyY(&temp2, &temp1);
    colorants[6] = temp2.x;
    colorants[7] = temp2.y;
    colorants[8] = temp2.Y;
    
    return colorants;
}

QVector <double> LcmsColorProfileContainer::getWhitePointXYZ() const
{
    QVector <double> tempWhitePoint(3);
    
    tempWhitePoint[0] = d->mediaWhitePoint.X;
    tempWhitePoint[1] = d->mediaWhitePoint.Y;
    tempWhitePoint[2] = d->mediaWhitePoint.Z;
    
    return tempWhitePoint;
}

QVector <double> LcmsColorProfileContainer::getWhitePointxyY() const
{
    QVector <double> tempWhitePoint(3);
    tempWhitePoint[0] = d->whitePoint.x;
    tempWhitePoint[1] = d->whitePoint.y;
    tempWhitePoint[2] = d->whitePoint.Y;
    return tempWhitePoint;
}



QVector <double> LcmsColorProfileContainer::getEstimatedTRC() const
{
    QVector <double> TRCtriplet(3);
    if (d->hasColorants) {
        if (cmsIsToneCurveLinear(d->redTRC)) {
            TRCtriplet[0] = 1.0;
        } else {
            TRCtriplet[0] = cmsEstimateGamma(d->redTRC, 0.01);
        }
        if (cmsIsToneCurveLinear(d->greenTRC)) {
            TRCtriplet[1] = 1.0;
        } else {
            TRCtriplet[1] = cmsEstimateGamma(d->greenTRC, 0.01);
        }
        if (cmsIsToneCurveLinear(d->blueTRC)) {
            TRCtriplet[2] = 1.0;
        } else {
            TRCtriplet[2] = cmsEstimateGamma(d->blueTRC, 0.01);
        }
            
    } else {
        if (cmsIsTag(d->profile, cmsSigGrayTRCTag)) {
            if (cmsIsToneCurveLinear(d->grayTRC)) {
                TRCtriplet.fill(1.0);
            } else {
                TRCtriplet.fill(cmsEstimateGamma(d->grayTRC,  0.01));
            } 
        } else {
            TRCtriplet.fill(1.0);
        }
    }
    return TRCtriplet;
}

QString LcmsColorProfileContainer::name() const
{
    return d->name;
}

QString LcmsColorProfileContainer::info() const
{
    return d->productDescription;
}
