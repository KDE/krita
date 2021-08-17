/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
                             KoSnapGuide *snapGuide,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction);
    ~KisMeshTransformStrategy() override;


    void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive, bool altModifierActive) override;
    QPointF handleSnapPoint(const QPointF &imagePos);
    bool shiftModifierIsUsed() const;

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
    void requestImageRecalculation();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MESH_TRANSFORM_STRATEGY_H */
