/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Pierre Ducroquet <pinaraf@pinaraf.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KoUnit.h>
#include "KoShadowStyle.h"

// KoShadowStyle private class
class KoShadowStylePrivate: public QSharedData
{
public:
    KoShadowStylePrivate();
    ~KoShadowStylePrivate();

    QList<KoShadowStyle::ShadowData> shadows;
};

KoShadowStylePrivate::KoShadowStylePrivate()
{
}

KoShadowStylePrivate::~KoShadowStylePrivate()
{
}

// KoShadowStyle::ShadowData structure
KoShadowStyle::ShadowData::ShadowData()
    : color(), offset(0, 0), radius(0.0)
{
}

bool KoShadowStyle::ShadowData::operator==(const KoShadowStyle::ShadowData &other) const
{
    return (color == other.color) && (offset == other.offset) && (radius == other.radius);
}

// KoShadowStyle class
KoShadowStyle::KoShadowStyle()
    : d(new KoShadowStylePrivate)
{
}

KoShadowStyle::KoShadowStyle(const KoShadowStyle &other)
    : d(other.d)
{
}

KoShadowStyle::~KoShadowStyle()
{
}

bool KoShadowStyle::operator==(const KoShadowStyle &other) const
{
    if (d.data() == other.d.data())
        return true;

    if (shadowCount() != other.shadowCount())
        return false;

    foreach (ShadowData data, d->shadows)
    {
        if (!other.d->shadows.contains(data))
            return false;
    }
    return true;
}

bool KoShadowStyle::operator!=(const KoShadowStyle &other) const
{
    return !operator==(other);
}

bool KoShadowStyle::loadOdf (const QString &data)
{
    if (data == "none")
        return true;
    QList<KoShadowStyle::ShadowData> shadow_datas;

    QStringList sub_shadows = data.split(',');
    foreach (QString shadow, sub_shadows) {
        QStringList words = shadow.split(' ', QString::SkipEmptyParts);
        if (words.length() == 0)
            return false;

        KoShadowStyle::ShadowData currentData;
        QColor shadowColor(words[0]);
        if (shadowColor.isValid()) {
            currentData.color = shadowColor;
            words.removeFirst();
        } else {
            // We keep an invalid color.
        }

        if (words.length() > 0) {
            if ((words.length() < 2) || (words.length() > 3))
                return false;

            // Parse the 2/3 lengths
            currentData.offset.setX(KoUnit::parseValue(words[0], 0.0));
            currentData.offset.setY(KoUnit::parseValue(words[1], 0.0));
            if (words.length() == 3)
                currentData.radius = KoUnit::parseValue(words[2], 0.0);
        }
        d->shadows << currentData;
    }
    return true;
}

int KoShadowStyle::shadowCount() const
{
    return d->shadows.length();
}

QString KoShadowStyle::saveOdf() const
{
    QStringList parts;
    foreach (ShadowData data, d->shadows) {
        QString part = QString("%1 %2pt %3pt %4pt").arg(data.color.name()).arg(data.offset.x()).arg(data.offset.y()).arg(data.radius);
        parts << part;
    }
    if (parts.isEmpty())
        return "none";
    return parts.join(",");
}

