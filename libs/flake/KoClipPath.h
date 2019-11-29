/* This file is part of the KDE project
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

#ifndef KOCLIPPATH_H
#define KOCLIPPATH_H

#include "kritaflake_export.h"

#include <QScopedPointer>
#include <QList>
#include <QSharedDataPointer>
#include <qnamespace.h>
#include <KoFlakeCoordinateSystem.h>

class KoShape;
class KoPathShape;
class QPainter;
class QTransform;
class QPainterPath;
class QSizeF;

/// Clip path used to clip shapes
class KRITAFLAKE_EXPORT KoClipPath
{
public:

    /**
     * Create a new shape clipping using the given clip data
     * @param clipShapes define the clipping shapes, owned by KoClipPath!
     * @param coordinates shows if ObjectBoundingBox or UserSpaceOnUse coordinate
     *                    system is used.
     */
    KoClipPath(QList<KoShape*> clipShapes, KoFlake::CoordinateSystem coordinates);
    ~KoClipPath();

    KoClipPath *clone() const;

    KoFlake::CoordinateSystem coordinates() const;

    /// Sets the clip rule to be used for the clip path
    void setClipRule(Qt::FillRule clipRule);

    /// Returns the current clip rule
    Qt::FillRule clipRule() const;

    /// Returns the current clip path with coordinates in percent of the clipped shape size
    QPainterPath path() const;

    /// Returns the current clip path scaled to match the specified shape size
    QPainterPath pathForSize(const QSizeF &size) const;

    /// Returns the clip path shapes
    QList<KoPathShape*> clipPathShapes() const;

    QList<KoShape*> clipShapes() const;

    /**
     * Returns the transformation from the clip data path shapes to the
     * current document coordinates of the specified clipped shape.
     * If the specified clipped shape is null, the transformation 
     * from clip data path shapes to shape coordinates of the clipped shape 
     * at the time of creating this clip path is being returned.
     */
    QTransform clipDataTransformation(KoShape *clippedShape) const;

    /// Applies the clipping to the given painter
    static void applyClipping(KoShape *clippedShape, QPainter &painter);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOCLIPPATH_H
