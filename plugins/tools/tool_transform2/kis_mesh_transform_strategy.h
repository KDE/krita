/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_MESH_TRANSFORM_STRATEGY_H
#define __KIS_MESH_TRANSFORM_STRATEGY_H

#include <QObject>
#include <QScopedPointer>

#include "kis_simplified_action_policy_strategy.h"

class QPointF;
class QPainter;
class KisCoordinatesConverter;
class ToolTransformArgs;
class TransformTransactionProperties;
class QImage;


class KisMeshTransformStrategy : public KisSimplifiedActionPolicyStrategy
{
    Q_OBJECT
public:
    KisMeshTransformStrategy(const KisCoordinatesConverter *converter,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction);
    ~KisMeshTransformStrategy() override;


    void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive, bool altModifierActive) override;
    void paint(QPainter &gc) override;
    QCursor getCurrentCursor() const override;
    void externalConfigChanged() override;

    bool beginPrimaryAction(const QPointF &pt) override;
    void continuePrimaryAction(const QPointF &pt, bool shiftModifierActve, bool altModifierActive) override;
    bool endPrimaryAction() override;

    using KisSimplifiedActionPolicyStrategy::beginPrimaryAction;
    using KisSimplifiedActionPolicyStrategy::continuePrimaryAction;
    using KisSimplifiedActionPolicyStrategy::endPrimaryAction;

    bool acceptsClicks() const override;

private:
    bool splitHoveredSegment(const QPointF &pt);
    bool shouldDeleteNode(qreal distance, qreal param);
    void verifyExpectedMeshSize();

private:
    Q_PRIVATE_SLOT(m_d, void recalculateTransformations());

Q_SIGNALS:
    void requestCanvasUpdate();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MESH_TRANSFORM_STRATEGY_H */
