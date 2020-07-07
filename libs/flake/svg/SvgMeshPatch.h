/*
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
#ifndef SVGMESHPATCH_H
#define SVGMESHPATCH_H

#include <QColor>
#include <QPointF>
#include <QVector>
#include <QMap>
#include <QScopedPointer>

#include <KoPathShape.h>

struct SvgMeshStop {
    QColor color;
    QPointF point;

    SvgMeshStop()
    {}

    SvgMeshStop(QColor color, QPointF point)
        : color(color), point(point)
    {}

    bool isValid() { return color.isValid(); }
};


class SvgMeshPatch
{
public:
    /// Position of stop in the patch
    enum Type {
        Top = 1,
        Right,
        Bottom,
        Left,
        Size,
    };

    SvgMeshPatch(QPointF startingPoint);
    SvgMeshPatch(const SvgMeshPatch& other);

    SvgMeshStop getStop(Type type) const;

    /// Get a segment of the path in the meshpatch
    KoPathSegment getPathSegment(Type type) const;

    /// Get full (closed) meshpath
    KoPathShape* getPath() const;

    /// Get size swept by mesh in pts
    QSizeF size() const;

    /// Gets the curve passing through the middle of meshpatch
    KoPathSegment getMidCurve(bool isVertical) const;

    void subdivide(QVector<SvgMeshPatch*>& subdivided) const;

    int countPoints() const;

    QRectF boundingRect() const;

    /* Parses raw pathstr and adds path to the shape, if the path isn't
     * complete, it will have to be computed and given with pathIncomplete = true
     * (Ideal case for std::optional)
     */
    void addStop(const QString& pathStr, QColor color, Type edge, bool pathIncomplete = false, QPointF lastPoint = QPointF());

    /// Adds path to the shape
    void addStop(const QList<QPointF>& pathPoints, QColor color, Type edge);

    void setTransform(const QTransform& matrix);

private:
    /* Parses path and adds it to m_path and returns the last point of the curve/line
     * see also: SvgMeshPatch::addStop
     */
    QPointF parseMeshPath(const QString& path, bool pathIncomplete = false, const QPointF lastPoint = QPointF());
    const char* getCoord(const char* ptr, qreal& number);

private:
    bool m_newPath;

    /// This is the starting point for each path
    QPointF m_startingPoint;

    QMap<Type, SvgMeshStop> m_nodes;
    QScopedPointer<KoPathShape> m_path;
};

#endif // SVGMESHPATCH_H
