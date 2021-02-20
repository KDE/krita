/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
