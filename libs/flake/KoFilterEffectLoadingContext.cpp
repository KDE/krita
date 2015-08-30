/* This file is part of the KDE project
* Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "KoFilterEffectLoadingContext.h"

#include <QString>
#include <QRectF>
#include <QFileInfo>
#include <QDir>

class Q_DECL_HIDDEN KoFilterEffectLoadingContext::Private
{
public:
    Private()
        : convertFilterUnits(false), convertFilterPrimitiveUnits(false)
    {}
    QString basePath;
    QRectF shapeBound;
    bool convertFilterUnits;
    bool convertFilterPrimitiveUnits;
};

KoFilterEffectLoadingContext::KoFilterEffectLoadingContext(const QString &basePath)
    : d(new Private())
{
    d->basePath = basePath;
}

KoFilterEffectLoadingContext::~KoFilterEffectLoadingContext()
{
    delete d;
}

void KoFilterEffectLoadingContext::setShapeBoundingBox(const QRectF &shapeBound)
{
    d->shapeBound = shapeBound;
}

void KoFilterEffectLoadingContext::enableFilterUnitsConversion(bool enable)
{
    d->convertFilterUnits = enable;
}

void KoFilterEffectLoadingContext::enableFilterPrimitiveUnitsConversion(bool enable)
{
    d->convertFilterPrimitiveUnits = enable;
}

QPointF KoFilterEffectLoadingContext::convertFilterUnits(const QPointF &value) const
{
    if (!d->convertFilterUnits)
        return value;

    return QPointF(convertFilterUnitsX(value.x()), convertFilterUnitsY(value.y()));
}

qreal KoFilterEffectLoadingContext::convertFilterUnitsX(qreal value) const
{
    if (!d->convertFilterUnits)
        return value;

    return value / d->shapeBound.width();
}

qreal KoFilterEffectLoadingContext::convertFilterUnitsY(qreal value) const
{
    if (!d->convertFilterUnits)
        return value;

    return value / d->shapeBound.height();
}

QPointF KoFilterEffectLoadingContext::convertFilterPrimitiveUnits(const QPointF &value) const
{
    if (!d->convertFilterPrimitiveUnits)
        return value;

    return QPointF(convertFilterPrimitiveUnitsX(value.x()), convertFilterPrimitiveUnitsY(value.y()));
}

qreal KoFilterEffectLoadingContext::convertFilterPrimitiveUnitsX(qreal value) const
{
    if (!d->convertFilterPrimitiveUnits)
        return value;

    return value / d->shapeBound.width();
}

qreal KoFilterEffectLoadingContext::convertFilterPrimitiveUnitsY(qreal value) const
{
    if (!d->convertFilterPrimitiveUnits)
        return value;

    return value / d->shapeBound.height();
}

QString KoFilterEffectLoadingContext::pathFromHref(const QString &href) const
{
    QFileInfo info(href);
    if (! info.isRelative())
        return href;

    QFileInfo pathInfo(QFileInfo(d->basePath).filePath());

    QString relFile = href;
    while (relFile.startsWith(QLatin1String("../"))) {
        relFile.remove(0, 3);
        pathInfo.setFile(pathInfo.dir(), QString());
    }

    QString absFile = pathInfo.absolutePath() + '/' + relFile;

    return absFile;
}
