/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KOSHAPEMESHGRADIENTHANDLES_H_
#define __KOSHAPEMESHGRADIENTHANDLES_H_

#include <QPointF>
#include <KoFlake.h>

#include <SvgMeshGradient.h>

class KUndo2Command;

class KoShapeMeshGradientHandles {
public:
    struct Handle {
        enum Type {
            None,
            Corner,
            BezierHandle
        };

        enum Index {
            First = 1,
            Second
        };

        Handle() : type(None) {}
        Handle(Type t, const QPointF &p, int row, int col, SvgMeshPatch::Type segmentType, Index index = First)
            : type(t) , pos(p)
            , row(row) , col(col)
            , segmentType(segmentType)
            , index(index)
        {
        }

        SvgMeshPosition getPosition() const {
            return SvgMeshPosition {row, col, segmentType};
        }

        Type type {None};
        QPointF pos;
        int row {0};
        int col {0};
        SvgMeshPatch::Type segmentType {SvgMeshPatch::Top};
        Index index { First }; // first or the second bezier handle
    };

public:
    KoShapeMeshGradientHandles(KoFlake::FillVariant fillVariant, KoShape *shape);

    /// get all nodes in the mesh, don't use this for drawing the path but use path()
    QVector<Handle> handles() const;

    /// convenience method to get a handle by its position in the mesharray
    Handle getHandle(SvgMeshPosition position) const;

    KUndo2Command* moveGradientHandle(const Handle &handle, const QPointF &newPos);

    QPainterPath path() const;
    QVector<QPainterPath> getConnectedPath(const Handle &handle) const;

private:
    const SvgMeshGradient* gradient() const;

    /// get handles including the corner
    QVector<Handle> getHandles(const SvgMeshArray *mesharray,
                                SvgMeshPatch::Type type,
                                int row,
                                int col) const;

    // get only bezier handles
    QVector<Handle> getBezierHandles(const SvgMeshArray *mesharray,
                                     SvgMeshPatch::Type type,
                                     int row,
                                     int col) const;

    QTransform abosoluteTransformation(KoFlake::CoordinateSystem system) const;

private:
    KoFlake::FillVariant m_fillVariant {KoFlake::Fill};
    KoShape *m_shape {0};
};

#endif // __KOSHAPEMESHGRADIENTHANDLES_H_
