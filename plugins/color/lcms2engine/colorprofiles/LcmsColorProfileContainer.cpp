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

#include "kis_debug.h"

class LcmsColorProfileContainer::Private
{
public:

    cmsHPROFILE profile;
    cmsColorSpaceSignature colorSpaceSignature;
    cmsProfileClassSignature deviceClass;
    QString productDescription;
    QString manufacturer;
    QString copyright;
    QString name;
    float version;
    IccColorProfile::Data *data {0};
    bool valid {false};
    bool suitableForOutput {false};
    bool hasColorants;
    bool hasTRC;
    bool adaptedFromD50;
    cmsCIEXYZ mediaWhitePoint;
    cmsCIExyY whitePoint;
    cmsCIEXYZTRIPLE colorants;
    cmsToneCurve *redTRC {0};
    cmsToneCurve *greenTRC {0};
    cmsToneCurve *blueTRC {0};
    cmsToneCurve *grayTRC {0};
    cmsToneCurve *redTRCReverse {0};
    cmsToneCurve *greenTRCReverse {0};
    cmsToneCurve *blueTRCReverse {0};
    cmsToneCurve *grayTRCReverse {0};

    cmsUInt32Number defaultIntent;
    bool isPerceptualCLUT;
    bool isRelativeCLUT;
    bool isAbsoluteCLUT;
    bool isSaturationCLUT;
    bool isMatrixShaper;

    QByteArray uniqueId;
};

LcmsColorProfileContainer::LcmsColorProfileContainer()
    : d(new Private())
{
    d->profile = 0;
}

LcmsColorProfileContainer::LcmsColorProfileContainer(IccColorProfile::Data *data)
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

IccColorProfile *LcmsColorProfileContainer::createFromLcmsProfile(const cmsHPROFILE profile)
{
    IccColorProfile *iccprofile = new IccColorProfile(lcmsProfileToByteArray(profile));
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
    if (d->profile) {
        cmsCloseProfile(d->profile);
    }

    d->profile = cmsOpenProfileFromMem((void *)d->data->rawData().constData(), d->data->rawData().size());


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

        //apparently this should give us a localised string??? Not sure about this.
        cmsGetProfileInfo(d->profile, cmsInfoModel, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->productDescription = QString::fromWCharArray(buffer);

        cmsGetProfileInfo(d->profile, cmsInfoManufacturer, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->manufacturer = QString::fromWCharArray(buffer);

        cmsGetProfileInfo(d->profile, cmsInfoCopyright, cmsNoLanguage, cmsNoCountry, buffer, _BUFFER_SIZE_);
        d->copyright = QString::fromWCharArray(buffer);

        cmsProfileClassSignature profile_class;
        profile_class = cmsGetDeviceClass(d->profile);
        d->valid = (profile_class != cmsSigNamedColorClass);

        //This is where obtain the whitepoint, and convert it to the actual white point of the profile in the case a Chromatic adaption tag is
        //present. This is necessary for profiles following the v4 spec.
        cmsCIEXYZ baseMediaWhitePoint;//dummy to hold copy of mediawhitepoint if this is modified by chromatic adaption.
        if (cmsIsTag(d->profile, cmsSigMediaWhitePointTag)) {
            d->mediaWhitePoint = *((cmsCIEXYZ *)cmsReadTag(d->profile, cmsSigMediaWhitePointTag));
            baseMediaWhitePoint = d->mediaWhitePoint;
            cmsXYZ2xyY(&d->whitePoint, &d->mediaWhitePoint);

            if (cmsIsTag(d->profile, cmsSigChromaticAdaptationTag)) {
                //the chromatic adaption tag represent a matrix from the actual white point of the profile to D50.
                cmsCIEXYZ *CAM1 = (cmsCIEXYZ *)cmsReadTag(d->profile, cmsSigChromaticAdaptationTag);
                //We first put all our data into structures we can manipulate.
                double d3dummy [3] = {d->mediaWhitePoint.X, d->mediaWhitePoint.Y, d->mediaWhitePoint.Z};
                QGenericMatrix<1, 3, double> whitePointMatrix(d3dummy);
                QTransform invertDummy(CAM1[0].X, CAM1[0].Y, CAM1[0].Z, CAM1[1].X, CAM1[1].Y, CAM1[1].Z, CAM1[2].X, CAM1[2].Y, CAM1[2].Z);
                //we then abuse QTransform's invert function because it probably does matrix invertion 20 times better than I can program.
                //if the matrix is uninvertable, invertedDummy will be an identity matrix, which for us means that it won't give any noticeble
                //effect when we start multiplying.
                QTransform invertedDummy = invertDummy.inverted();
                //we then put the QTransform into a generic 3x3 matrix.
                double d9dummy [9] = {invertedDummy.m11(), invertedDummy.m12(), invertedDummy.m13(),
                                      invertedDummy.m21(), invertedDummy.m22(), invertedDummy.m23(),
                                      invertedDummy.m31(), invertedDummy.m32(), invertedDummy.m33()
                                     };
                QGenericMatrix<3, 3, double> chromaticAdaptionMatrix(d9dummy);
                //multiplying our inverted adaption matrix with the whitepoint gives us the right whitepoint.
                QGenericMatrix<1, 3, double> result = chromaticAdaptionMatrix * whitePointMatrix;
                //and then we pour the matrix into the whitepoint variable. Generic matrix does row/column for indices even though it
                //uses column/row for initialising.
                d->mediaWhitePoint.X = result(0, 0);
                d->mediaWhitePoint.Y = result(1, 0);
                d->mediaWhitePoint.Z = result(2, 0);
                cmsXYZ2xyY(&d->whitePoint, &d->mediaWhitePoint);
            }
        }
        //This is for RGB profiles, but it only works for matrix profiles. Need to design it to work with non-matrix profiles.
        if (cmsIsTag(d->profile, cmsSigRedColorantTag)) {
            cmsCIEXYZTRIPLE tempColorants;
            tempColorants.Red = *((cmsCIEXYZ *)cmsReadTag(d->profile, cmsSigRedColorantTag));
            tempColorants.Green = *((cmsCIEXYZ *)cmsReadTag(d->profile, cmsSigGreenColorantTag));
            tempColorants.Blue = *((cmsCIEXYZ *)cmsReadTag(d->profile, cmsSigBlueColorantTag));
            //convert to d65, this is useless.
            cmsAdaptToIlluminant(&d->colorants.Red, &baseMediaWhitePoint, &d->mediaWhitePoint, &tempColorants.Red);
            cmsAdaptToIlluminant(&d->colorants.Green, &baseMediaWhitePoint, &d->mediaWhitePoint, &tempColorants.Green);
            cmsAdaptToIlluminant(&d->colorants.Blue, &baseMediaWhitePoint, &d->mediaWhitePoint, &tempColorants.Blue);
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
            if (d->redTRC) d->redTRCReverse = cmsReverseToneCurve(d->redTRC);
            if (d->greenTRC) d->greenTRCReverse = cmsReverseToneCurve(d->greenTRC);
            if (d->blueTRC) d->blueTRCReverse = cmsReverseToneCurve(d->blueTRC);
            d->hasTRC = (d->redTRC && d->greenTRC && d->blueTRC && d->redTRCReverse && d->greenTRCReverse && d->blueTRCReverse);

        } else if (cmsIsTag(d->profile, cmsSigGrayTRCTag)) {
            d->grayTRC = ((cmsToneCurve *)cmsReadTag (d->profile, cmsSigGrayTRCTag));
            if (d->grayTRC) d->grayTRCReverse = cmsReverseToneCurve(d->grayTRC);
            d->hasTRC = (d->grayTRC && d->grayTRCReverse);
        } else {
            d->hasTRC = false;
        }

        // Check if the profile can convert (something->this)
        d->suitableForOutput = cmsIsMatrixShaper(d->profile)
                               || (cmsIsCLUT(d->profile, INTENT_PERCEPTUAL, LCMS_USED_AS_INPUT) &&
                                   cmsIsCLUT(d->profile, INTENT_PERCEPTUAL, LCMS_USED_AS_OUTPUT));

        d->version = cmsGetProfileVersion(d->profile);
        d->defaultIntent = cmsGetHeaderRenderingIntent(d->profile);
        d->isMatrixShaper = cmsIsMatrixShaper(d->profile);
        d->isPerceptualCLUT = cmsIsCLUT(d->profile, INTENT_PERCEPTUAL, LCMS_USED_AS_INPUT);
        d->isSaturationCLUT = cmsIsCLUT(d->profile, INTENT_SATURATION, LCMS_USED_AS_INPUT);
        d->isAbsoluteCLUT = cmsIsCLUT(d->profile, INTENT_SATURATION, LCMS_USED_AS_INPUT);
        d->isRelativeCLUT = cmsIsCLUT(d->profile, INTENT_RELATIVE_COLORIMETRIC, LCMS_USED_AS_INPUT);

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

QString LcmsColorProfileContainer::copyright() const
{
    return d->copyright;
}

bool LcmsColorProfileContainer::valid() const
{
    return d->valid;
}

float LcmsColorProfileContainer::version() const
{
    return d->version;
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

bool LcmsColorProfileContainer::supportsPerceptual() const
{
    return d->isPerceptualCLUT;
}
bool LcmsColorProfileContainer::supportsSaturation() const
{
    return d->isSaturationCLUT;
}
bool LcmsColorProfileContainer::supportsAbsolute() const
{
    return d->isAbsoluteCLUT;//LCMS2 doesn't convert matrix shapers via absolute intent, because of V4 workflow.
}
bool LcmsColorProfileContainer::supportsRelative() const
{
    if (d->isRelativeCLUT || d->isMatrixShaper){
        return true;
    }
    return false;
}
bool LcmsColorProfileContainer::hasColorants() const
{
    return d->hasColorants;
}
bool LcmsColorProfileContainer::hasTRC() const
{
    return d->hasTRC;
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

void LcmsColorProfileContainer::LinearizeFloatValue(QVector <double> & Value) const
{
    if (d->hasColorants) {
        if (!cmsIsToneCurveLinear(d->redTRC)) {
            Value[0] = cmsEvalToneCurveFloat(d->redTRC, Value[0]);
        }
        if (!cmsIsToneCurveLinear(d->greenTRC)) {
            Value[1] = cmsEvalToneCurveFloat(d->greenTRC, Value[1]);
        }
        if (!cmsIsToneCurveLinear(d->blueTRC)) {
            Value[2] = cmsEvalToneCurveFloat(d->blueTRC, Value[2]);
        }

    } else {
        if (cmsIsTag(d->profile, cmsSigGrayTRCTag)) {
            Value[0] = cmsEvalToneCurveFloat(d->grayTRC, Value[0]);
        }
    }
}

void LcmsColorProfileContainer::DelinearizeFloatValue(QVector <double> & Value) const
{
    if (d->hasColorants) {
        if (!cmsIsToneCurveLinear(d->redTRC)) {
            Value[0] = cmsEvalToneCurveFloat(d->redTRCReverse, Value[0]);
        }
        if (!cmsIsToneCurveLinear(d->greenTRC)) {
            Value[1] = cmsEvalToneCurveFloat(d->greenTRCReverse, Value[1]);
        }
        if (!cmsIsToneCurveLinear(d->blueTRC)) {
            Value[2] = cmsEvalToneCurveFloat(d->blueTRCReverse, Value[2]);
        }

    } else {
        if (cmsIsTag(d->profile, cmsSigGrayTRCTag)) {
            Value[0] = cmsEvalToneCurveFloat(d->grayTRCReverse, Value[0]);
        }
    }
}

void LcmsColorProfileContainer::LinearizeFloatValueFast(QVector <double> & Value) const
{
    const qreal scale = 65535.0;
    const qreal invScale = 1.0 / scale;

    if (d->hasColorants) {
        //we can only reliably delinearise in the 0-1.0 range, outside of that leave the value alone.
        QVector <quint16> TRCtriplet(3);
        TRCtriplet[0] = Value[0] * scale;
        TRCtriplet[1] = Value[1] * scale;
        TRCtriplet[2] = Value[2] * scale;

        if (!cmsIsToneCurveLinear(d->redTRC) && Value[0]<1.0) {
            TRCtriplet[0] = cmsEvalToneCurve16(d->redTRC, TRCtriplet[0]);
            Value[0] = TRCtriplet[0] * invScale;
        }
        if (!cmsIsToneCurveLinear(d->greenTRC) && Value[1]<1.0) {
            TRCtriplet[1] = cmsEvalToneCurve16(d->greenTRC, TRCtriplet[1]);
            Value[1] = TRCtriplet[1] * invScale;
        }
        if (!cmsIsToneCurveLinear(d->blueTRC) && Value[2]<1.0) {
            TRCtriplet[2] = cmsEvalToneCurve16(d->blueTRC, TRCtriplet[2]);
            Value[2] = TRCtriplet[2] * invScale;
        }
    } else {
        if (cmsIsTag(d->profile, cmsSigGrayTRCTag) && Value[0]<1.0) {
            quint16 newValue = cmsEvalToneCurve16(d->grayTRC, Value[0] * scale);
            Value[0] = newValue * invScale;
        }
    }
}
void LcmsColorProfileContainer::DelinearizeFloatValueFast(QVector <double> & Value) const
{
    const qreal scale = 65535.0;
    const qreal invScale = 1.0 / scale;

    if (d->hasColorants) {
        //we can only reliably delinearise in the 0-1.0 range, outside of that leave the value alone.
        QVector <quint16> TRCtriplet(3);
        TRCtriplet[0] = Value[0] * scale;
        TRCtriplet[1] = Value[1] * scale;
        TRCtriplet[2] = Value[2] * scale;

        if (!cmsIsToneCurveLinear(d->redTRC) && Value[0]<1.0) {
            TRCtriplet[0] = cmsEvalToneCurve16(d->redTRCReverse, TRCtriplet[0]);
            Value[0] = TRCtriplet[0] * invScale;
        }
        if (!cmsIsToneCurveLinear(d->greenTRC) && Value[1]<1.0) {
            TRCtriplet[1] = cmsEvalToneCurve16(d->greenTRCReverse, TRCtriplet[1]);
            Value[1] = TRCtriplet[1] * invScale;
        }
        if (!cmsIsToneCurveLinear(d->blueTRC) && Value[2]<1.0) {
            TRCtriplet[2] = cmsEvalToneCurve16(d->blueTRCReverse, TRCtriplet[2]);
            Value[2] = TRCtriplet[2] * invScale;
        }
    } else {
        if (cmsIsTag(d->profile, cmsSigGrayTRCTag) && Value[0]<1.0) {
            quint16 newValue = cmsEvalToneCurve16(d->grayTRCReverse, Value[0] * scale);
            Value[0] = newValue * invScale;
        }
    }
}

QString LcmsColorProfileContainer::name() const
{
    return d->name;
}

QString LcmsColorProfileContainer::info() const
{
    return d->productDescription;
}

QByteArray LcmsColorProfileContainer::getProfileUniqueId() const
{
    if (d->uniqueId.isEmpty() && d->profile) {
        QByteArray id(sizeof(cmsProfileID), 0);
        cmsGetHeaderProfileID(d->profile, (quint8*)id.data());

        bool isNull = std::all_of(id.constBegin(),
                                  id.constEnd(),
                                  [](char c) {return c == 0;});
        if (isNull) {
            if (cmsMD5computeID(d->profile)) {
                cmsGetHeaderProfileID(d->profile, (quint8*)id.data());
                isNull = false;
            }
        }

        if (!isNull) {
            d->uniqueId = id;
        }
    }

    return d->uniqueId;
}
