/* This file is part of the KDE project
 * 
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "KoConnectionPoint.h"

KoConnectionPoint::KoConnectionPoint()
: position(0, 0), escapeDirection(AllDirections), alignment(AlignNone)
{
}

KoConnectionPoint::KoConnectionPoint(const QPointF& pos)
: position(pos), escapeDirection(AllDirections), alignment(AlignNone)
{
}

KoConnectionPoint::KoConnectionPoint(const QPointF& pos, EscapeDirection direction)
: position(pos), escapeDirection(direction), alignment(AlignNone)
{
}

KoConnectionPoint::KoConnectionPoint(const QPointF &pos, EscapeDirection direction, Alignment alignment)
: position(pos), escapeDirection(direction), alignment(alignment)
{
}

KoConnectionPoint KoConnectionPoint::defaultConnectionPoint(PointId connectionPointId)
{
    switch(connectionPointId)
    {
        case TopConnectionPoint:
            return KoConnectionPoint(QPointF(0.5, 0.0));
        case RightConnectionPoint:
            return KoConnectionPoint(QPointF(1.0, 0.5));
        case BottomConnectionPoint:
            return KoConnectionPoint(QPointF(0.5, 1.0));
        case LeftConnectionPoint:
            return KoConnectionPoint(QPointF(0.0, 0.5));
        default:
            return KoConnectionPoint();
    }
}
