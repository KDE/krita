/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

