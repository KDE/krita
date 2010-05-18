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
#ifndef INTERACTIONSTRATEGY_H
#define INTERACTIONSTRATEGY_H

#include <QPointF>


/**
 * Baseclass for interaction handling.
 * The shape selector allows mouse and keyboard interaction by having the Canvas
 * use different InteractionStrategy classes to handle an 'interaction'
 * (mouse down till mouse release)
 */
class InteractionStrategy
{
public:
    InteractionStrategy() {}
    virtual ~InteractionStrategy() {}

    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) = 0;
    virtual void cancelInteraction() { }
    virtual void finishInteraction( Qt::KeyboardModifiers modifiers ) { Q_UNUSED(modifiers); }
};

#endif
