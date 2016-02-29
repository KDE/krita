/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoGuidesData.h"
#include "KoViewConverter.h"
#include <KoUnit.h>
#include <KoOasisSettings.h>
#include <KoXmlWriter.h>

#include <QPainter>

class Q_DECL_HIDDEN KoGuidesData::Private
{
public:
    Private()
        : showGuides(false),
        snapToGuides(false),
        lockGuides(false),
        guidesColor(Qt::lightGray) {}

    bool operator==(const Private &rhs) {
        return horzGuideLines == rhs.horzGuideLines &&
            vertGuideLines == rhs.vertGuideLines &&
            showGuides == rhs.showGuides &&
            snapToGuides == rhs.snapToGuides &&
            lockGuides == rhs.lockGuides &&
            guidesColor == rhs.guidesColor;
    }

    QList<qreal> horzGuideLines;
    QList<qreal> vertGuideLines;

    bool showGuides;
    bool snapToGuides;
    bool lockGuides;

    QColor guidesColor;
};

KoGuidesData::KoGuidesData()
    : d(new Private())
{
}

KoGuidesData::~KoGuidesData()
{
    delete d;
}

KoGuidesData::KoGuidesData(const KoGuidesData &rhs)
    : d(new Private(*rhs.d))
{
}

KoGuidesData& KoGuidesData::operator=(const KoGuidesData &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }

    return *this;
}

bool KoGuidesData::operator==(const KoGuidesData &rhs) const
{
    return *d == *rhs.d;
}

void KoGuidesData::setHorizontalGuideLines(const QList<qreal> &lines)
{
    d->horzGuideLines = lines;
}

void KoGuidesData::setVerticalGuideLines(const QList<qreal> &lines)
{
    d->vertGuideLines = lines;
}

void KoGuidesData::addGuideLine(Qt::Orientation o, qreal pos)
{
    if (o == Qt::Horizontal) {
        d->horzGuideLines.append(pos);
    } else {
        d->vertGuideLines.append(pos);
    }
}

bool KoGuidesData::showGuideLines() const
{
    return d->showGuides;
}

void KoGuidesData::setShowGuideLines(bool show)
{
    d->showGuides = show;
}

bool KoGuidesData::showGuides() const
{
    return d->showGuides;
}

void KoGuidesData::setShowGuides(bool value)
{
    d->showGuides = value;
}

bool KoGuidesData::lockGuides() const
{
    return d->lockGuides;
}

void KoGuidesData::setLockGuides(bool value)
{
    d->lockGuides = value;
}

bool KoGuidesData::snapToGuides() const
{
    return d->snapToGuides;
}

void KoGuidesData::setSnapToGuides(bool value)
{
    d->snapToGuides = value;
}

const QList<qreal>& KoGuidesData::horizontalGuideLines() const
{
    return d->horzGuideLines;
}

const QList<qreal>& KoGuidesData::verticalGuideLines() const
{
    return d->vertGuideLines;
}

bool KoGuidesData::hasGuides() const
{
    return !d->horzGuideLines.isEmpty() || !d->vertGuideLines.isEmpty();
}

void KoGuidesData::setGuidesColor(const QColor &color)
{
    d->guidesColor = color;
}

QColor KoGuidesData::guidesColor() const
{
    return d->guidesColor;
}
