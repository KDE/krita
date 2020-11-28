/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoColorConversionTransformationFactory.h"

#include <QString>

#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "DebugPigment.h"
#include "KoColorSpaceRegistry.h"

struct Q_DECL_HIDDEN KoColorConversionTransformationFactory::Private {
    QString srcModelId;
    QString srcDepthId;
    QString dstModelId;
    QString dstDepthId;
    QString srcProfile;
    QString dstProfile;
};

KoColorConversionTransformationFactory::KoColorConversionTransformationFactory(const QString &_srcModelId, const QString &_srcDepthId, const QString &_srcProfile, const QString &_dstModelId, const QString &_dstDepthId, const QString &_dstProfile) : d(new Private)
{
    d->srcModelId = _srcModelId;
    d->srcDepthId = _srcDepthId;
    d->dstModelId = _dstModelId;
    d->dstDepthId = _dstDepthId;
    d->srcProfile = KoColorSpaceRegistry::instance()->profileAlias(_srcProfile);
    d->dstProfile = KoColorSpaceRegistry::instance()->profileAlias(_dstProfile);
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

