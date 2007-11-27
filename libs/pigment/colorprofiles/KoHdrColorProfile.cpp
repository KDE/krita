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

#include "KoIccColorProfile.h"

struct KoHdrColorProfile::Private {
    KoIccColorProfile* iccProfile;
    double exposure;
};

KoHdrColorProfile::KoHdrColorProfile(const QString &name, const QString &info) : d(new Private)
{
    d->iccProfile = 0;
    d->exposure = 0.0;
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

double KoHdrColorProfile::hdrExposure() const
{
    return d->exposure;
}

void KoHdrColorProfile::setHdrExposure(double exposure)
{
    d->exposure = exposure;
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
