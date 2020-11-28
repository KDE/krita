/* This file is part of the KDE project
* SPDX-FileCopyrightText: 2010 Jan Hambrecht <jaham@gmx.net>
*
* SPDX-License-Identifier: LGPL-2.1-or-later
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
