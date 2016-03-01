/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>

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

#include "kis_guides_config.h"

#include <QDomDocument>
#include "kis_dom_utils.h"


class Q_DECL_HIDDEN KisGuidesConfig::Private
{
public:
    Private()
        : showGuides(false),
        snapToGuides(false),
        lockGuides(false) {}

    bool operator==(const Private &rhs) {
        return horzGuideLines == rhs.horzGuideLines &&
            vertGuideLines == rhs.vertGuideLines &&
            showGuides == rhs.showGuides &&
            snapToGuides == rhs.snapToGuides &&
            lockGuides == rhs.lockGuides;
    }

    QList<qreal> horzGuideLines;
    QList<qreal> vertGuideLines;

    bool showGuides;
    bool snapToGuides;
    bool lockGuides;
};

KisGuidesConfig::KisGuidesConfig()
    : d(new Private())
{
}

KisGuidesConfig::~KisGuidesConfig()
{
}

KisGuidesConfig::KisGuidesConfig(const KisGuidesConfig &rhs)
    : d(new Private(*rhs.d))
{
}

KisGuidesConfig& KisGuidesConfig::operator=(const KisGuidesConfig &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }

    return *this;
}

bool KisGuidesConfig::operator==(const KisGuidesConfig &rhs) const
{
    return *d == *rhs.d;
}

void KisGuidesConfig::setHorizontalGuideLines(const QList<qreal> &lines)
{
    d->horzGuideLines = lines;
}

void KisGuidesConfig::setVerticalGuideLines(const QList<qreal> &lines)
{
    d->vertGuideLines = lines;
}

void KisGuidesConfig::addGuideLine(Qt::Orientation o, qreal pos)
{
    if (o == Qt::Horizontal) {
        d->horzGuideLines.append(pos);
    } else {
        d->vertGuideLines.append(pos);
    }
}

bool KisGuidesConfig::showGuideLines() const
{
    return d->showGuides;
}

void KisGuidesConfig::setShowGuideLines(bool show)
{
    d->showGuides = show;
}

bool KisGuidesConfig::showGuides() const
{
    return d->showGuides;
}

void KisGuidesConfig::setShowGuides(bool value)
{
    d->showGuides = value;
}

bool KisGuidesConfig::lockGuides() const
{
    return d->lockGuides;
}

void KisGuidesConfig::setLockGuides(bool value)
{
    d->lockGuides = value;
}

bool KisGuidesConfig::snapToGuides() const
{
    return d->snapToGuides;
}

void KisGuidesConfig::setSnapToGuides(bool value)
{
    d->snapToGuides = value;
}

const QList<qreal>& KisGuidesConfig::horizontalGuideLines() const
{
    return d->horzGuideLines;
}

const QList<qreal>& KisGuidesConfig::verticalGuideLines() const
{
    return d->vertGuideLines;
}

bool KisGuidesConfig::hasGuides() const
{
    return !d->horzGuideLines.isEmpty() || !d->vertGuideLines.isEmpty();
}

QDomElement KisGuidesConfig::saveToXml(QDomDocument& doc, const QString &tag) const
{
    QDomElement gridElement = doc.createElement(tag);
    KisDomUtils::saveValue(&gridElement, "showGuides", d->showGuides);
    KisDomUtils::saveValue(&gridElement, "snapToGuides", d->snapToGuides);
    KisDomUtils::saveValue(&gridElement, "lockGuides", d->lockGuides);

    KisDomUtils::saveValue(&gridElement, "horizontalGuides", d->horzGuideLines.toVector());
    KisDomUtils::saveValue(&gridElement, "verticalGuides", d->vertGuideLines.toVector());

    return gridElement;
}

bool KisGuidesConfig::loadFromXml(const QDomElement &parent)
{
    bool result = true;

    result &= KisDomUtils::loadValue(parent, "showGuides", &d->showGuides);
    result &= KisDomUtils::loadValue(parent, "snapToGuides", &d->snapToGuides);
    result &= KisDomUtils::loadValue(parent, "lockGuides", &d->lockGuides);

    QVector<qreal> hGuides;
    QVector<qreal> vGuides;

    result &= KisDomUtils::loadValue(parent, "horizontalGuides", &hGuides);
    result &= KisDomUtils::loadValue(parent, "verticalGuides", &vGuides);

    d->horzGuideLines = QList<qreal>::fromVector(hGuides);
    d->vertGuideLines = QList<qreal>::fromVector(vGuides);

    return result;
}
