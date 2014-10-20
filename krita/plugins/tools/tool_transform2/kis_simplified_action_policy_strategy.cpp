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

#include "kis_simplified_action_policy_strategy.h"

#include "KoPointerEvent.h"
#include "kis_coordinates_converter.h"


struct KisSimplifiedActionPolicyStrategy::Private
{
    Private(const KisCoordinatesConverter *_converter)
        : converter(_converter),
          changeSizeModifierActive(false),
          anyPickerModifierActive(false) {}

    const KisCoordinatesConverter *converter;

    bool changeSizeModifierActive;
    bool anyPickerModifierActive;
};


KisSimplifiedActionPolicyStrategy::KisSimplifiedActionPolicyStrategy(const KisCoordinatesConverter *_converter)
    : m_d(new Private(_converter))
{
}

KisSimplifiedActionPolicyStrategy::~KisSimplifiedActionPolicyStrategy()
{
}

bool KisSimplifiedActionPolicyStrategy::beginPrimaryAction(KoPointerEvent *event)
{
    return beginPrimaryAction(m_d->converter->documentToImage(event->point));
}

void KisSimplifiedActionPolicyStrategy::continuePrimaryAction(KoPointerEvent *event)
{
    continuePrimaryAction(m_d->converter->documentToImage(event->point), false);
}

void KisSimplifiedActionPolicyStrategy::hoverActionCommon(KoPointerEvent *event)
{
    hoverActionCommon(m_d->converter->documentToImage(event->point));
}

bool KisSimplifiedActionPolicyStrategy::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    return endPrimaryAction();
}

void KisSimplifiedActionPolicyStrategy::activateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::ChangeSize) {
        m_d->changeSizeModifierActive = true;
    } else if (action == KisTool::PickFgNode || action == KisTool::PickBgNode ||
               action == KisTool::PickFgImage || action == KisTool::PickBgImage) {

        m_d->anyPickerModifierActive = true;
    }
}

void KisSimplifiedActionPolicyStrategy::deactivateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::ChangeSize) {
        m_d->changeSizeModifierActive = false;
    } else if (action == KisTool::PickFgNode || action == KisTool::PickBgNode ||
               action == KisTool::PickFgImage || action == KisTool::PickBgImage) {

        m_d->anyPickerModifierActive = false;
    }
}

bool KisSimplifiedActionPolicyStrategy::beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(action);

    if (!m_d->changeSizeModifierActive && !m_d->anyPickerModifierActive) return false;

    return beginPrimaryAction(m_d->converter->documentToImage(event->point));
}

void KisSimplifiedActionPolicyStrategy::continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(action);

    if (!m_d->changeSizeModifierActive && !m_d->anyPickerModifierActive) return;

    continuePrimaryAction(m_d->converter->documentToImage(event->point), m_d->changeSizeModifierActive);
}

bool KisSimplifiedActionPolicyStrategy::endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(action);

    if (!m_d->changeSizeModifierActive && !m_d->anyPickerModifierActive) return false;

    return endPrimaryAction();
}

void KisSimplifiedActionPolicyStrategy::hoverActionCommon(const QPointF &pt)
{
    setTransformFunction(pt, m_d->anyPickerModifierActive);
}

