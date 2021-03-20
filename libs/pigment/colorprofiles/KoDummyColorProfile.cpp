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
    QVector<double> d50Dummy(3);
    d50Dummy<<0.34773<<0.35952<<1.0;
    return d50Dummy;
}

QVector<double> KoDummyColorProfile::getColorantsxyY() const
{
    QVector<double> d50Dummy(3);
    d50Dummy<<0.34773<<0.35952<<1.0;
    return d50Dummy;
}

QVector<double> KoDummyColorProfile::getWhitePointXYZ() const
{
    QVector<double> d50Dummy(3);
    d50Dummy<<0.9642<<1.0000<<0.8249;
    return d50Dummy;
}

QVector<double> KoDummyColorProfile::getWhitePointxyY() const
{
    QVector<double> d50Dummy(3);
    d50Dummy<<0.34773<<0.35952<<1.0;
    return d50Dummy;
}

QVector <double> KoDummyColorProfile::getEstimatedTRC() const

{
    QVector<double> Dummy(3);
    Dummy.fill(2.2);
    return Dummy;
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

