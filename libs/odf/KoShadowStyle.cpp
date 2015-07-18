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

#include "KoShadowStyle.h"

#include <KoUnit.h>


// KoShadowStyle private class
class KoShadowStylePrivate: public QSharedData
{
public:
    KoShadowStylePrivate();
    ~KoShadowStylePrivate();

    QVector<KoShadowStyle::ShadowData> shadows;
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

    foreach (const ShadowData &data, d->shadows)
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

// load value string as specified by CSS2 §7.16.5 "text-shadow"
bool KoShadowStyle::loadOdf (const QString &data)
{
    if (data == QLatin1String("none"))
        return true;

    const QStringList sub_shadows = data.split(QLatin1Char(','));
    foreach (const QString &shadow, sub_shadows) {
        QStringList words = shadow.split(QLatin1Char(' '), QString::SkipEmptyParts);
        if (words.isEmpty())
            return false;

        KoShadowStyle::ShadowData currentData;

        // look for color at begin
        QColor shadowColor(words.first());
        if (shadowColor.isValid()) {
            currentData.color = shadowColor;
            words.removeFirst();
        } else if (words.length() > 2) {
            // look for color at end, if there could be one
            shadowColor = QColor(words.last());
            if (shadowColor.isValid()) {
                currentData.color = shadowColor;
                words.removeLast();
            }
        }
        // We keep an invalid color.if none was found

        // "Each shadow effect must specify a shadow offset and may optionally
        // specify a blur radius and a shadow color.", from CSS2 §7.16.5 "text-shadow"
        // But for some reason also no offset has been accepted before. TODO: which?
        if (! words.isEmpty()) {
            if ((words.length() < 2) || (words.length() > 3))
                return false;

            // Parse offset
            currentData.offset.setX(KoUnit::parseValue(words.at(0), 0.0));
            currentData.offset.setY(KoUnit::parseValue(words.at(1), 0.0));
            // Parse blur radius if present
            if (words.length() == 3)
                currentData.radius = KoUnit::parseValue(words.at(2), 0.0);
        }
        d->shadows << currentData;
    }
    return true;
}

int KoShadowStyle::shadowCount() const
{
    return d->shadows.size();
}

QString KoShadowStyle::saveOdf() const
{
    if (d->shadows.isEmpty())
        return QLatin1String("none");

    QStringList parts;
    const QString pt = QLatin1String("%1pt");
    foreach (const ShadowData &data, d->shadows) {
        QStringList elements;
        if (data.color.isValid()) {
            elements << data.color.name();
        }
        elements << pt.arg(data.offset.x()) << pt.arg(data.offset.y());
        if (data.radius != 0)
            elements << pt.arg(data.radius);

        parts << elements.join(QLatin1String(" "));
    }
    return parts.join(QLatin1String(","));
}

