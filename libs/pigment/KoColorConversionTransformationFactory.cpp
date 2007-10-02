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

#include "KoColorSpace.h"

struct KoColorConversionTransformationFactory::Private {
    QString srcModelId;
    QString srcDepthId;
    QString dstModelId;
    QString dstDepthId;
};

KoColorConversionTransformationFactory::KoColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _dstModelId, QString _dstDepthId) : d(new Private)
{
    d->srcModelId = _srcModelId;
    d->srcDepthId = _srcDepthId;
    d->dstModelId = _dstModelId;
    d->dstDepthId = _dstDepthId;
}

KoColorConversionTransformationFactory::~KoColorConversionTransformationFactory()
{
    delete d;
}

bool KoColorConversionTransformationFactory::canBeSource(KoColorSpace* srcCS)
{
    return ((srcCS->colorModelId().id() == d->srcModelId) && (srcCS->colorDepthId().id() == d->srcDepthId));
}

bool KoColorConversionTransformationFactory::canBeDestination(KoColorSpace* dstCS)
{
    return ((dstCS->colorModelId().id() == d->dstModelId) && (dstCS->colorDepthId().id() == d->dstDepthId));
}

QString KoColorConversionTransformationFactory::srcColorModelId() const
{
    return d->srcModelId;
}
QString KoColorConversionTransformationFactory::srcColorDepthId() const
{
    return d->srcDepthId;
}
QString KoColorConversionTransformationFactory::dstColorModelId() const
{
    return d->dstModelId;
}
QString KoColorConversionTransformationFactory::dstColorDepthId() const
{
    return d->dstDepthId;
}
