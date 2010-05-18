/*
 * Copyright (C) 2008-2010 Thomas Zander <zander@kde.org>
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
#ifndef SELECTICONSTRATEGY_H
#define SELECTICONSTRATEGY_H

#include "InteractionStrategy.h"

#include <QObject>

class Canvas;
class KoShape;
class KoPointerEvent;

/**
 * A strategy for handling a left click on a selectable shape.
 * The shape selector allows mouse and keyboard interaction by having the Canvas
 * use different InteractionStrategy classes to handle an 'interaction'
 * (mouse down till mouse release)
 * This is the strategy that detects the user selecting an item that can be inserted
 * onto a KOffice canvas.
 * itemSelected will be emitted after the canvas' KoSelection has been updated to
 * select the appropriate shape.
 */
class SelectStrategy : public QObject, public InteractionStrategy
{
    Q_OBJECT
public:
    SelectStrategy(Canvas *canvas, KoShape *clickedShape, KoPointerEvent &event);

    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual void finishInteraction( Qt::KeyboardModifiers modifiers );

signals:
    void itemSelected();

private:
    Canvas *m_canvas;
    KoShape *m_clickedShape;
    bool m_emitItemSelected;
};

#endif
