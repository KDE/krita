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

        Handle() : type(None) {}
        Handle(Type t, const QPointF &p, int row, int col, SvgMeshPatch::Type segmentType)
            : type(t) , pos(p)
            , row(row) , col(col)
            , segmentType(segmentType)
        {
        }

        Type type;
        QPointF pos;
        int row, col;
        SvgMeshPatch::Type segmentType;
    };

public:
    KoShapeMeshGradientHandles(KoFlake::FillVariant fillVariant, KoShape *shape);

    // get all nodes in the mesh
    QVector<QVector<Handle>> handles() const;

    KUndo2Command* moveGradientHandle(const Handle &handle, const QPointF &newPos);

    QPainterPath path() const;

private:
    const SvgMeshGradient* gradient() const;

    int getHandleIndex(const std::array<QPointF, 4> &path, QPointF point);

    QVector<Handle> toHandles(const SvgMeshArray *mesharray,
                              SvgMeshPatch::Type type,
                              int row,
                              int col) const;

    QTransform abosoluteTransformation(KoFlake::CoordinateSystem system) const;

private:
    KoFlake::FillVariant m_fillVariant;
    KoShape *m_shape;
};

#endif // __KOSHAPEMESHGRADIENTHANDLES_H_
