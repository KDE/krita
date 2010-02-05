/* This file is part of the KDE project
* Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
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

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

class KoFilterEffectLoadingContext::Private
{
public:
    QString basePath;
    QRectF shapeBound;
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

QPointF KoFilterEffectLoadingContext::fromUserSpace(const QPointF &value) const
{
    return QPointF(fromUserSpaceX(value.x()), fromUserSpaceY(value.y()));
}

qreal KoFilterEffectLoadingContext::fromUserSpaceX(qreal value) const
{
    if (!d->shapeBound.width())
        return value;

    return value / d->shapeBound.width();
}

qreal KoFilterEffectLoadingContext::fromUserSpaceY(qreal value) const
{
    if (!d->shapeBound.height())
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
    while (relFile.startsWith("../")) {
        relFile = relFile.mid(3);
        pathInfo.setFile(pathInfo.dir(), QString());
    }

    QString absFile = pathInfo.absolutePath() + '/' + relFile;

    return absFile;
}
