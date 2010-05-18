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
#ifndef DRAGCANVASSTRATEGY_H
#define DRAGCANVASSTRATEGY_H

#include "InteractionStrategy.h"

class Canvas;
class KoPointerEvent;

/**
 * A strategy for handling dragging the mouse to drag the canvas around.
 * The shape selector allows mouse and keyboard interaction by having the Canvas
 * use different InteractionStrategy classes to handle an 'interaction'
 * (mouse down till mouse release)
 * This is the strategy that allows moving the whole canvas around when its too
 * big for the viewport.
 */
class DragCanvasStrategy : public InteractionStrategy
{
public:
    DragCanvasStrategy(Canvas *canvas, KoPointerEvent &event);

    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);

private:
    Canvas *m_canvas;
    QPointF m_lastPosition;
};

#endif
