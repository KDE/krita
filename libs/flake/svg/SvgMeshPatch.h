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
    ~SvgMeshPatch();

    SvgMeshStop* getStop(Type type) const;

    int countPoints() const;

    void parseStop(const QString& pathStr, QColor color, int row, int col);

private:
    void parseMeshPath(const QString path, bool close = false);
    const char* getCoord(const char* ptr, qreal& number);

private:
    bool m_newPath;

    /// This is the starting point for each path
    QPointF m_startingPoint;

    QMap<Type, SvgMeshStop*> m_nodes;
    QScopedPointer<KoPathShape> m_path;
};

#endif // SVGMESHPATCH_H
