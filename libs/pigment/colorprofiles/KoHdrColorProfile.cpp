/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "KoHdrColorProfile.h"

#include <math.h>

#include "KoIccColorProfile.h"

struct KoHdrColorProfile::Private {
    KoIccColorProfile* iccProfile;
    qreal exposure;
    qreal exposureFactor;
    qreal invExposureFactor;
    qreal middleGreyScaleFactor;
};

KoHdrColorProfile::KoHdrColorProfile(const QString &name, const QString &info) : d(new Private)
{
    d->iccProfile = 0;

    // After adjusting by the exposure, map 1.0 to 3.5 f-stops below 1.0
    // I.e. scale by 1/(2^3.5).
    d->middleGreyScaleFactor = 0.0883883;

    setHdrExposure(0.0);
    setName(name);
    setInfo(info);
}

KoHdrColorProfile::KoHdrColorProfile(const KoHdrColorProfile& rhs) : KoColorProfile(rhs), d(new Private(*rhs.d))
{
    if(d->iccProfile)
    {
        d->iccProfile = (KoIccColorProfile*)d->iccProfile->clone();
    }
}

KoHdrColorProfile::~KoHdrColorProfile()
{
    delete d->iccProfile;
    delete d;
}

const KoIccColorProfile* KoHdrColorProfile::iccProfile() const
{
    return d->iccProfile;
}

void KoHdrColorProfile::setIccColorProfile(KoIccColorProfile* profile)
{
    d->iccProfile = profile;
}

KoColorProfile* KoHdrColorProfile::clone() const
{
    return new KoHdrColorProfile(*this);
}

bool KoHdrColorProfile::valid() const
{
    return true;
}

bool KoHdrColorProfile::isSuitableForOutput() const
{
    return true;
}

bool KoHdrColorProfile::isSuitableForPrinting() const
{
    return true;
}

bool KoHdrColorProfile::isSuitableForDisplay() const
{
    return true;
}

qreal KoHdrColorProfile::hdrExposure() const
{
    return d->exposure;
}

void KoHdrColorProfile::setHdrExposure(qreal exposure)
{
    d->exposure = exposure;
    d->exposureFactor = pow(2, exposure + 2.47393) * d->middleGreyScaleFactor* 65535.0;
    d->invExposureFactor = 1.0 / d->exposureFactor;
}

bool KoHdrColorProfile::operator==(const KoColorProfile& rhs) const
{
    const KoHdrColorProfile* rhsHdr = dynamic_cast<const KoHdrColorProfile*>(&rhs);
    if(rhsHdr)
    {
        return *iccProfile() == *rhsHdr->iccProfile();
    }
    return false;
}

quint16 KoHdrColorProfile::channelToDisplay(qreal value) const
{
    value *= d->exposureFactor;

    const int minU16 = 0;
    const int maxU16 = 65535;

    return (quint16)qBound(minU16, qRound(value), maxU16);
}

qreal KoHdrColorProfile::displayToChannel(quint16 value) const
{
    return value * d->invExposureFactor;
}

qreal KoHdrColorProfile::channelToDisplayDouble(qreal value) const
{
    value = value * d->exposureFactor / 0xFFFF;

    const qreal min = 0;
    const qreal max = 1;

    return qBound(min, value, max);
}

qreal KoHdrColorProfile::displayToChannelDouble(qreal value) const
{
    return value * 0xFFFF * d->invExposureFactor;
}

QVariant KoHdrColorProfile::property( const QString& _name) const
{
    if( _name == "exposure" )
    {
        return d->exposure;
    } else {
        return KoColorProfile::property( _name );
    }
}
void KoHdrColorProfile::setProperty( const QString& _name, const QVariant& _variant)
{
    if( _name == "exposure" )
    {
        setHdrExposure( _variant.toDouble() );
    } else {
        KoColorProfile::setProperty( _name, _variant );
    }
}
