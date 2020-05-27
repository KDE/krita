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

int KoShadowStyle::shadowCount() const
{
    return d->shadows.size();
}

