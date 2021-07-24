/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FREE_TRANSFORM_STRATEGY_H
#define __KIS_FREE_TRANSFORM_STRATEGY_H

#include <QObject>
#include <QScopedPointer>

#include "kis_simplified_action_policy_strategy.h"

class QPointF;
class QPainter;
class KisCoordinatesConverter;
class ToolTransformArgs;
class TransformTransactionProperties;
class QCursor;

class KisFreeTransformStrategy : public KisSimplifiedActionPolicyStrategy
{
    Q_OBJECT
public:
    KisFreeTransformStrategy(const KisCoordinatesConverter *converter,
                             KoSnapGuide *snapGuide,
                             ToolTransformArgs &currentArgs,
                             TransformTransactionProperties &transaction);
    ~KisFreeTransformStrategy() override;

    void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive, bool altModifierActive) override;
    bool shiftModifierIsUsed() const;

    void paint(QPainter &gc) override;
    QCursor getCurrentCursor() const override;

    void externalConfigChanged() override;

    using KisTransformStrategyBase::beginPrimaryAction;
    using KisTransformStrategyBase::continuePrimaryAction;
    using KisTransformStrategyBase::endPrimaryAction;

    bool beginPrimaryAction(const QPointF &pt) override;
    void continuePrimaryAction(const QPointF &pt, bool shiftModifierActve, bool altModifierActive) override;
    bool endPrimaryAction() override;

Q_SIGNALS:
    void requestCanvasUpdate();
    void requestResetRotationCenterButtons();
    void requestShowImageTooBig(bool value);
    void requestImageRecalculation();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_FREE_TRANSFORM_STRATEGY_H */
