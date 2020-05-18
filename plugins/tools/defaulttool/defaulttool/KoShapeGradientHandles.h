/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOSHAPEGRADIENTHANDLES_H
#define KOSHAPEGRADIENTHANDLES_H

#include <QPointF>
#include <QGradient>
#include <KoFlake.h>

class KoShape;
class KoViewConverter;
class KUndo2Command;

class KoShapeGradientHandles
{
public:
    struct Handle {
        enum Type {
            None,
            LinearStart,
            LinearEnd,
            RadialCenter,
            RadialRadius,
            RadialFocalPoint
        };

        Handle() : type(None) {}
        Handle(Type t, const QPointF &p) : type(t), pos(p) {}

        Type type;
        QPointF pos;
    };

public:
    KoShapeGradientHandles(KoFlake::FillVariant fillVariant, KoShape *shape);
    QVector<Handle> handles() const;
    QGradient::Type type() const;

    KUndo2Command* moveGradientHandle(Handle::Type handleType, const QPointF &absoluteOffset);
    Handle getHandle(Handle::Type handleType);



private:
    const QGradient* gradient() const;
    QPointF getNewHandlePos(const QPointF &oldPos, const QPointF &absoluteOffset, QGradient::CoordinateMode mode);

private:
    KoFlake::FillVariant m_fillVariant;
    KoShape *m_shape;
};

#endif // KOSHAPEGRADIENTHANDLES_H
