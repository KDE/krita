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
#include <KoSnapGuide.h>
#include "kis_coordinates_converter.h"


struct KisSimplifiedActionPolicyStrategy::Private
{
    Private(const KisCoordinatesConverter *_converter, KoSnapGuide *_snapGuide)
        : converter(_converter),
          snapGuide(_snapGuide),
          changeSizeModifierActive(false),
          anyPickerModifierActive(false) {}

    const KisCoordinatesConverter *converter;
    KoSnapGuide *snapGuide;

    bool changeSizeModifierActive;
    bool anyPickerModifierActive;
    QPointF dragOffset;
};


KisSimplifiedActionPolicyStrategy::KisSimplifiedActionPolicyStrategy(const KisCoordinatesConverter *_converter, KoSnapGuide *_snapGuide)
    : m_d(new Private(_converter, _snapGuide))
{
}

KisSimplifiedActionPolicyStrategy::~KisSimplifiedActionPolicyStrategy()
{
}

QPointF KisSimplifiedActionPolicyStrategy::handleSnapPoint(const QPointF &imagePos)
{
    return imagePos;
}

bool KisSimplifiedActionPolicyStrategy::beginPrimaryAction(KoPointerEvent *event)
{
    const QPointF rawImagePoint = m_d->converter->documentToImage(event->point);
    const QPointF snappedImagePoint = handleSnapPoint(rawImagePoint);

    /**
     * Note: Snapping with Offset is not yet used in the transform
     *       strategies.  When the user starts an action, we just move
     *       the handle to the mouse position, even if it was
     *       positioned with an offset.  That is not what we do in
     *       Crop Tool.
     */
    if (m_d->snapGuide && rawImagePoint != snappedImagePoint) {
        QPointF imageOffset = snappedImagePoint - rawImagePoint;
        m_d->dragOffset = m_d->converter->imageToDocument(imageOffset);
    }

    const QPointF pos =
        m_d->snapGuide ?
        m_d->snapGuide->snap(event->point, m_d->dragOffset, event->modifiers()) :
        event->point;

    return beginPrimaryAction(m_d->converter->documentToImage(pos));
}

void KisSimplifiedActionPolicyStrategy::continuePrimaryAction(KoPointerEvent *event)
{
    /**
     * HACK ALERT!
     *
     * Here we explicitly check for Shift key pressed! The chioce of
     * the stroke type is usually done before the tablet press, but
     * for some actions like constrain proportions we should be able
     * to activate it even after the strokehas been started. For now,
     * KisShortcutMatcher does not support it, so just hardcode this
     * special case.
     *
     * See bug 340496
     */
    const bool shiftIsActive = event->modifiers() & Qt::ShiftModifier;

    const QPointF pos =
        m_d->snapGuide ?
        m_d->snapGuide->snap(event->point, m_d->dragOffset, event->modifiers()) :
        event->point;

    return continuePrimaryAction(m_d->converter->documentToImage(pos), shiftIsActive);
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
    Q_UNUSED(event);
    Q_UNUSED(action);

    if (!m_d->changeSizeModifierActive && !m_d->anyPickerModifierActive) return false;

    return endPrimaryAction();
}

void KisSimplifiedActionPolicyStrategy::hoverActionCommon(const QPointF &pt)
{
    setTransformFunction(pt, m_d->anyPickerModifierActive);
}

