/* This file is part of the KDE project
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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
#ifndef KOPATHSHAPEPRIVATE_H
#define KOPATHSHAPEPRIVATE_H

#include "KoShape_p.h"

class KoPathShapePrivate : public KoShapePrivate
{
public:
    KoPathShapePrivate(KoPathShape *q);

    QRectF handleRect(const QPointF &p, qreal radius) const;
    /// Applies the viewbox transformation defined in the given element
    void applyViewboxTransformation(const KoXmlElement &element);

    Qt::FillRule fillRule;

    Q_DECLARE_PUBLIC(KoPathShape)
};

#endif
