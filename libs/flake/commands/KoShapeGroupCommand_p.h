/* This file is part of the KDE project
 * Copyright (C) 2006,2009,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006,2007 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KoShapeGroupCommandPrivate_H
#define KoShapeGroupCommandPrivate_H

#include <QTransform>
#include <QPair>

class KoShapeGroupCommandPrivate
{
public:
    KoShapeGroupCommandPrivate(KoShapeContainer *container, const QList<KoShape *> &shapes, const QList<bool> &clipped = QList<bool>(), const QList<bool> &inheritTransform = QList<bool>());
    void init(QUndoCommand *q);
    QRectF containerBoundingRect();

    QList<KoShape*> shapes; ///<list of shapes to be grouped
    QList<bool> clipped; ///< list of booleans to specify the shape of the same index to be clipped
    QList<bool> inheritTransform; ///< list of booleans to specify the shape of the same index to inherit transform
    KoShapeContainer *container; ///< the container where the grouping should be for.
    QList<KoShapeContainer*> oldParents; ///< the old parents of the shapes
    QList<bool> oldClipped; ///< if the shape was clipped in the old parent
    QList<bool> oldInheritTransform; ///< if the shape was inheriting transform in the old parent
    QList<int> oldZIndex; ///< the old z-index of the shapes

    QList<QPair<KoShape*, int> > oldAncestorsZIndex; // only used by the ungroup command
};

#endif
