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

#include "KoColorConversionTransformationFactory.h"

#include <QString>

#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "DebugPigment.h"

struct KoColorConversionTransformationFactory::Private {
    QString srcModelId;
    QString srcDepthId;
    QString dstModelId;
    QString dstDepthId;
    QString srcProfile;
    QString dstProfile;
};

KoColorConversionTransformationFactory::KoColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _srcProfile, QString _dstModelId, QString _dstDepthId, QString _dstProfile) : d(new Private)
{
    d->srcModelId = _srcModelId;
    d->srcDepthId = _srcDepthId;
    d->dstModelId = _dstModelId;
    d->dstDepthId = _dstDepthId;
    d->srcProfile = _srcProfile;
    d->dstProfile = _dstProfile;
}

KoColorConversionTransformationFactory::~KoColorConversionTransformationFactory()
{
    delete d;
}

bool KoColorConversionTransformationFactory::canBeSource(const KoColorSpace* srcCS) const
{
    return ((srcCS->colorModelId().id() == d->srcModelId)
            && (srcCS->colorDepthId().id() == d->srcDepthId)
            && (d->srcProfile == "" || srcCS->profile()->name() == d->srcProfile));
}

bool KoColorConversionTransformationFactory::canBeDestination(const KoColorSpace* dstCS) const
{
    dbgPigment << dstCS->colorModelId().id() << " " << d->dstModelId << " " << dstCS->colorDepthId().id() << " " <<  d->dstDepthId << " " << d->dstProfile << " " << (dstCS->profile() ? dstCS->profile()->name() : "noprofile")  << " " << d->dstProfile;
    return ((dstCS->colorModelId().id() == d->dstModelId)
            && (dstCS->colorDepthId().id() == d->dstDepthId)
            && (d->dstProfile == "" || dstCS->profile()->name() == d->dstProfile));
}

QString KoColorConversionTransformationFactory::srcColorModelId() const
{
    return d->srcModelId;
}
QString KoColorConversionTransformationFactory::srcColorDepthId() const
{
    return d->srcDepthId;
}

QString KoColorConversionTransformationFactory::srcProfile() const
{
    return d->srcProfile;
}

QString KoColorConversionTransformationFactory::dstColorModelId() const
{
    return d->dstModelId;
}
QString KoColorConversionTransformationFactory::dstColorDepthId() const
{
    return d->dstDepthId;
}

QString KoColorConversionTransformationFactory::dstProfile() const
{
    return d->dstProfile;
}

