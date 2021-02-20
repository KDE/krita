/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SHAPESHEARSTRATEGY_H
#define SHAPESHEARSTRATEGY_H

#include <KoInteractionStrategy.h>
#include <KoFlake.h>

#include <QPointF>
#include <QSizeF>
#include <QTransform>

class KoToolBase;
class KoShape;
class KoSelection;

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
    ShapeShearStrategy(KoToolBase *tool, KoSelection *selection, const QPointF &clicked, KoFlake::SelectionHandle direction);
    ~ShapeShearStrategy() override {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override
    {
        Q_UNUSED(modifiers);
    }
    void paint(QPainter &painter, const KoViewConverter &converter) override;

private:
    QPointF m_start;
    QPointF m_solidPoint;
    QSizeF m_initialSize;
    bool m_top, m_left, m_bottom, m_right;
    qreal m_initialSelectionAngle;
    QTransform m_shearMatrix;
    bool m_isMirrored;
    QList<QTransform> m_oldTransforms;
    QList<KoShape *> m_transformedShapesAndSelection;
};

#endif

