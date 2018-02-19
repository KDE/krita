/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISHANDLEUTILILTYTYPES_H
#define KISHANDLEUTILILTYTYPES_H

#include "kritaglobal_export.h"

#include <boost/variant.hpp>
#include <QPointF>
#include <QPainterPath>
#include "KisHandleStyle.h"


namespace KritaUtils {

enum HandlePointType {
    Invalid = 0,
    Rect,
    Diamond,
    GradientDiamond,
    Circle,
    SmallCircle,
    GradientCross
};

enum HandleLineType {
    ConnectionLine,
    GradientArrow
};

enum HandlePathType {
    OutlinePath
};

struct KRITAGLOBAL_EXPORT Handle {
    struct PointHandle {
        HandlePointType type;
        QPointF pos;
    };
    struct LineHandle {
        HandleLineType type;
        QPointF p1;
        QPointF p2;
    };
    struct PathHandle {
        HandlePathType type;
        QPainterPath path;
    };



    Handle() : value (PointHandle({Invalid, QPointF()})) {}
    Handle(HandlePointType type, const QPointF &pos) : value(PointHandle({type, pos})) {}
    Handle(HandleLineType lineType, const QPointF &p1, const QPointF &p2) : value(LineHandle({lineType, p1, p2})) {}
    Handle(HandlePathType pathType, const QPainterPath &path) : value(PathHandle({pathType, path})) {}
    Handle(HandlePathType pathType, const QPolygonF &polygon);

    boost::variant<PointHandle, LineHandle, PathHandle> value;
};

typedef QVector<KritaUtils::Handle> HandlesVector;

}

#endif // KISHANDLEUTILILTYTYPES_H
