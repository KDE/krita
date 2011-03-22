/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@kogmbh.com>
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

#include "KoTextLayoutRootArea.h"

#include <KoShape.h>


class KoTextLayoutRootArea::Private
{
public:
    Private()
        : shape(0)
        , dirty(true)
    {
    }
    KoShape *shape;
    bool dirty;
};

KoTextLayoutRootArea::KoTextLayoutRootArea()
  : KoTextLayoutArea(0)
  , d(new Private)
{
}

KoTextLayoutRootArea::~KoTextLayoutRootArea()
{
}

void KoTextLayoutRootArea::layout(HierarchicalCursor *cursor)
{
    Q_UNUSED(cursor);
    KoTextLayoutArea::layout(cursor);
    d->dirty = false;
}

void KoTextLayoutRootArea::setAssociatedShape(KoShape *shape)
{
    d->shape = shape;
}

KoShape *KoTextLayoutRootArea::associatedShape()
{
    return d->shape;
}

void KoTextLayoutRootArea::setDirty()
{
    d->dirty = true;
}

bool KoTextLayoutRootArea::isDirty()
{
    return d->dirty;
}

qreal KoTextLayoutRootArea::maximalAllowedY() const
{
    return 500;
}

KoText::Direction KoTextLayoutRootArea::parentTextDirection() const
{
    return KoText::LeftRightTopBottom;
}

qreal KoTextLayoutRootArea::left() const
{
    return 0.0;
}

qreal KoTextLayoutRootArea::right() const
{
    return d->shape->size().width();
}
