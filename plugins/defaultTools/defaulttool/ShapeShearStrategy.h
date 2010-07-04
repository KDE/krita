/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef SHAPESHEARSTRATEGY_H
#define SHAPESHEARSTRATEGY_H

#include <KoInteractionStrategy.h>
#include <KoFlake.h>

#include <QPointF>
#include <QSizeF>
#include <QTransform>

class KoCanvasBase;
class KoToolBase;
class KoShape;

/**
 * A strategy for the KoInteractionTool.
 * This strategy is invoked when the user starts a shear of a selection of objects,
 * the stategy will then shear the objects interactively and provide a command afterwards.
 */
class ShapeShearStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor that starts to rotate the objects.
     * @param tool the parent tool which controls this strategy
     * @param clicked the initial point that the user depressed (in pt).
     * @param direction the handle that was grabbed
     */
    ShapeShearStrategy( KoToolBase *tool, const QPointF &clicked, KoFlake::SelectionHandle direction );
    virtual ~ShapeShearStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    QUndoCommand* createCommand();
    void finishInteraction( Qt::KeyboardModifiers modifiers ) { Q_UNUSED( modifiers ); }
    virtual void paint( QPainter &painter, const KoViewConverter &converter);

private:
    QPointF m_start;
    QPointF m_solidPoint;
    QSizeF m_initialSize;
    bool m_top, m_left, m_bottom, m_right;
    qreal m_initialSelectionAngle;
    QTransform m_shearMatrix;
    bool m_isMirrored;
    QList<QTransform> m_oldTransforms;
    QTransform m_initialSelectionMatrix;
    QList<KoShape*> m_selectedShapes;
};

#endif

