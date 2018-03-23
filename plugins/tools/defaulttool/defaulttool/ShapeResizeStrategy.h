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

#ifndef SHAPERESIZESTRATEGY_H
#define SHAPERESIZESTRATEGY_H

#include <KoInteractionStrategy.h>
#include <KoFlake.h>

#include <QScopedPointer>
#include <QPointF>
#include <QList>
#include <QTransform>

class KoToolBase;
class KoShape;
class KoShapeResizeCommand;
class KoSelection;

/**
 * A strategy for the KoInteractionTool.
 * This strategy is invoked when the user starts a resize of a selection of objects,
 * the stategy will then resize the objects interactively and provide a command afterwards.
 */
class ShapeResizeStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor
     */
    ShapeResizeStrategy(KoToolBase *tool, KoSelection *selection, const QPointF &clicked, KoFlake::SelectionHandle direction, bool forceUniformScalingMode);
    ~ShapeResizeStrategy() override;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;
private:
    void resizeBy(const QPointF &stillPoint, qreal zoomX, qreal zoomY);

    QPointF m_start;
    QList<KoShape *> m_selectedShapes;

    QTransform m_postScalingCoveringTransform;
    QSizeF m_initialSelectionSize;
    QTransform m_unwindMatrix;
    bool m_top, m_left, m_bottom, m_right;

    QPointF m_globalStillPoint;
    QPointF m_globalCenterPoint;
    QScopedPointer<KoShapeResizeCommand> m_executedCommand;

    bool m_forceUniformScalingMode = false;
};

#endif

