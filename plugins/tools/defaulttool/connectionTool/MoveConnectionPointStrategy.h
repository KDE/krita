/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef MOVECONNECTIONPOINTSTRATEGY_H
#define MOVECONNECTIONPOINTSTRATEGY_H

#include <KoInteractionStrategy.h>
#include <KoConnectionPoint.h>
#include <QPointF>

class KoShape;

class MoveConnectionPointStrategy : public KoInteractionStrategy
{
public:
    /// Constructor
    MoveConnectionPointStrategy(KoShape *shape, int connectionPointId, KoToolBase *parent);
    /// Destructor
    virtual ~MoveConnectionPointStrategy();

    /// reimplemented from KoInteractionStrategy
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented from KoInteractionStrategy
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);

    /// reimplemented from KoInteractionStrategy
    virtual KUndo2Command *createCommand();

    /// reimplemented from KoInteractionStrategy
    virtual void cancelInteraction();

    /// reimplemented from KoInteractionStrategy
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);

private:
    KoShape *m_shape;
    int m_connectionPointId;
    KoConnectionPoint m_oldPoint;
    KoConnectionPoint m_newPoint;
};

#endif // MOVECONNECTIONPOINTSTRATEGY_H
