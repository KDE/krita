/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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
    ~KisSimplifiedActionPolicyStrategy();

    virtual bool beginPrimaryAction(KoPointerEvent *event);
    virtual void continuePrimaryAction(KoPointerEvent *event);
    virtual bool endPrimaryAction(KoPointerEvent *event);
    virtual void hoverActionCommon(KoPointerEvent *event);

    virtual QPointF handleSnapPoint(const QPointF &imagePos);

    virtual void activateAlternateAction(KisTool::AlternateAction action);
    virtual void deactivateAlternateAction(KisTool::AlternateAction action);

    virtual bool beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action);
    virtual void continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action);
    virtual bool endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action);

protected:

    virtual void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive) = 0;

    virtual bool beginPrimaryAction(const QPointF &pt) = 0;
    virtual void continuePrimaryAction(const QPointF &pt, bool specialModifierActve) = 0;
    virtual bool endPrimaryAction() = 0;
    virtual void hoverActionCommon(const QPointF &pt);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SIMPLIFIED_ACTION_POLICY_STRATEGY_H */
