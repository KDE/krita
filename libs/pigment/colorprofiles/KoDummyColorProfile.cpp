/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoDummyColorProfile.h"

KoDummyColorProfile::KoDummyColorProfile()
{
    setName("default");
}

KoDummyColorProfile::~KoDummyColorProfile()
{
}

KoColorProfile* KoDummyColorProfile::clone() const
{
    return new KoDummyColorProfile();
}

bool KoDummyColorProfile::valid() const
{
    return true;
}
float KoDummyColorProfile::version() const
{
    return 0.0;
}
bool KoDummyColorProfile::isSuitableForOutput() const
{
    return true;
}
bool KoDummyColorProfile::isSuitableForInput() const
{
    return true;
}
bool KoDummyColorProfile::isSuitableForWorkspace() const
{
    return true;
}

bool KoDummyColorProfile::isSuitableForPrinting() const
{
    return true;
}

bool KoDummyColorProfile::isSuitableForDisplay() const
{
    return true;
}
bool KoDummyColorProfile::supportsPerceptual() const
{
    return true;
}
bool KoDummyColorProfile::supportsSaturation() const
{
    return true;
}
bool KoDummyColorProfile::supportsAbsolute() const
{
    return true;
}
bool KoDummyColorProfile::supportsRelative() const
{
    return true;
}
bool KoDummyColorProfile::hasColorants() const
{
    return true;
}
bool KoDummyColorProfile::hasTRC() const
{
    return true;
}
bool KoDummyColorProfile::isLinear() const
{
    return true;
}
QVector<double> KoDummyColorProfile::getColorantsXYZ() const
{
    QVector<double> sRGBStandardAdaptedColorants = {
        0.43603516, 0.22248840, 0.01391602,
        0.38511658, 0.71690369, 0.09706116,
        0.14305115, 0.06060791, 0.71392822
    };
    return sRGBStandardAdaptedColorants;
}

QVector<double> KoDummyColorProfile::getColorantsxyY() const
{
    QVector<double> result(9);
    auto srgb = getColorantsXYZ();
    result[0] = srgb[0] / (srgb[0] + srgb[1] + srgb[2]);
    result[1] = srgb[1] / (srgb[0] + srgb[1] + srgb[2]);
    result[2] = srgb[1];

    result[3] = srgb[3] / (srgb[3] + srgb[4] + srgb[5]);
    result[4] = srgb[4] / (srgb[3] + srgb[4] + srgb[5]);
    result[5] = srgb[4];

    result[6] = srgb[6] / (srgb[6] + srgb[7] + srgb[8]);
    result[7] = srgb[7] / (srgb[6] + srgb[7] + srgb[8]);
    result[8] = srgb[7];

    return result;
}

QVector<double> KoDummyColorProfile::getWhitePointXYZ() const
{
    QVector<double> d50Dummy;
    d50Dummy<<0.9642<<1.0000<<0.8249;
    return d50Dummy;
}

QVector<double> KoDummyColorProfile::getWhitePointxyY() const
{
    QVector<double> d50Dummy;
    d50Dummy<<0.34773<<0.35952<<1.0;
    return d50Dummy;
}

QVector <double> KoDummyColorProfile::getEstimatedTRC() const

{
    QVector<double> Dummy(3);
    Dummy.fill(2.2);
    return Dummy;
}

bool KoDummyColorProfile::compareTRC(TransferCharacteristics, float) const
{
    return false;
}

void KoDummyColorProfile::linearizeFloatValue(QVector <double> & ) const
{
}

void KoDummyColorProfile::delinearizeFloatValue(QVector <double> & ) const
{
}
void KoDummyColorProfile::linearizeFloatValueFast(QVector <double> & ) const
{
}

void KoDummyColorProfile::delinearizeFloatValueFast(QVector <double> & ) const
{
}
bool KoDummyColorProfile::operator==(const KoColorProfile& rhs) const
{
    return dynamic_cast<const KoDummyColorProfile*>(&rhs);
}

QByteArray KoDummyColorProfile::uniqueId() const
{
    return QByteArray();
}

