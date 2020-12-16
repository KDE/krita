/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SIMPLIFIED_ACTION_POLICY_STRATEGY_H
#define __KIS_SIMPLIFIED_ACTION_POLICY_STRATEGY_H

#include <QScopedPointer>

#include "kis_transform_strategy_base.h"


class KoPointerEvent;
class KisCoordinatesConverter;
class KoSnapGuide;

class KisSimplifiedActionPolicyStrategy : public KisTransformStrategyBase
{
public:
    KisSimplifiedActionPolicyStrategy(const KisCoordinatesConverter *_converter, KoSnapGuide *snapGuide = 0);
    ~KisSimplifiedActionPolicyStrategy() override;

    void activatePrimaryAction() override;

    bool beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    bool endPrimaryAction(KoPointerEvent *event) override;
    void hoverActionCommon(KoPointerEvent *event) override;

    virtual QPointF handleSnapPoint(const QPointF &imagePos);

    void activateAlternateAction(KisTool::AlternateAction action) override;
    void deactivateAlternateAction(KisTool::AlternateAction action) override;

    bool beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) override;
    bool endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) override;

protected:

    virtual void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive, bool shiftModifierActive, bool altModifierActive) = 0;

    virtual bool beginPrimaryAction(const QPointF &pt) = 0;
    virtual void continuePrimaryAction(const QPointF &pt, bool shiftModifierActve, bool altModifierActive) = 0;
    virtual bool endPrimaryAction() = 0;
    virtual void hoverActionCommon(const QPointF &pt);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SIMPLIFIED_ACTION_POLICY_STRATEGY_H */
