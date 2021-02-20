/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
          sampleFromNodeModifierActive(false),
          changeSizeModifierActive(false),
          anySamplerModifierActive(false) {}

    const KisCoordinatesConverter *converter;
    KoSnapGuide *snapGuide;

    bool sampleFromNodeModifierActive;
    bool changeSizeModifierActive;
    bool anySamplerModifierActive;
    QPointF dragOffset;
    QPointF lastImagePos;
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

    QPointF imagePos = m_d->converter->documentToImage(pos);
    m_d->lastImagePos = imagePos;

    return beginPrimaryAction(imagePos);
}

void KisSimplifiedActionPolicyStrategy::continuePrimaryAction(KoPointerEvent *event)
{
    /**
     * HACK ALERT!
     *
     * Here we explicitly check for Shift key pressed! The choice of
     * the stroke type is usually done before the tablet press, but
     * for some actions like constrain proportions we should be able
     * to activate it even after the stroke has been started. For now,
     * KisShortcutMatcher does not support it, so just hardcode this
     * special case.
     *
     * See bug 340496
     */
    const bool shiftIsActive = event->modifiers() & Qt::ShiftModifier;
    const bool altIsActive = event->modifiers() & Qt::AltModifier;

    const QPointF pos =
        m_d->snapGuide ?
        m_d->snapGuide->snap(event->point, m_d->dragOffset, event->modifiers()) :
        event->point;

    QPointF imagePos = m_d->converter->documentToImage(pos);
    m_d->lastImagePos = imagePos;

    return continuePrimaryAction(imagePos, shiftIsActive, altIsActive);
}

void KisSimplifiedActionPolicyStrategy::hoverActionCommon(KoPointerEvent *event)
{
    QPointF imagePos = m_d->converter->documentToImage(event->point);
    m_d->lastImagePos = imagePos;

    hoverActionCommon(imagePos);
}

bool KisSimplifiedActionPolicyStrategy::endPrimaryAction(KoPointerEvent *event)
{
    QPointF imagePos = m_d->converter->documentToImage(event->point);
    m_d->lastImagePos = imagePos;

    return endPrimaryAction();
}

void KisSimplifiedActionPolicyStrategy::activatePrimaryAction()
{
    setTransformFunction(m_d->lastImagePos, m_d->anySamplerModifierActive && !m_d->sampleFromNodeModifierActive, m_d->changeSizeModifierActive, m_d->sampleFromNodeModifierActive);
}

void KisSimplifiedActionPolicyStrategy::activateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::ChangeSize) {
        m_d->changeSizeModifierActive = true;
    } else if (action == KisTool::SampleFgNode || action == KisTool::SampleBgNode) {
        m_d->anySamplerModifierActive = true;
        m_d->sampleFromNodeModifierActive = true;
    } else if (action == KisTool::SampleFgImage || action == KisTool::SampleBgImage) {
        m_d->anySamplerModifierActive = true;
        m_d->sampleFromNodeModifierActive = false;
    }

    setTransformFunction(m_d->lastImagePos, m_d->anySamplerModifierActive && !m_d->sampleFromNodeModifierActive, m_d->changeSizeModifierActive, m_d->sampleFromNodeModifierActive);
}

void KisSimplifiedActionPolicyStrategy::deactivateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::ChangeSize) {
        m_d->changeSizeModifierActive = false;
    } else if (action == KisTool::SampleFgNode || action == KisTool::SampleBgNode ||
               action == KisTool::SampleFgImage || action == KisTool::SampleBgImage) {

        m_d->anySamplerModifierActive = false;
        m_d->sampleFromNodeModifierActive = false;
    }
}

bool KisSimplifiedActionPolicyStrategy::beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(action);

    if (!m_d->changeSizeModifierActive && !m_d->anySamplerModifierActive) return false;

    QPointF imagePos = m_d->converter->documentToImage(event->point);
    m_d->lastImagePos = imagePos;

    return beginPrimaryAction(imagePos);
}

void KisSimplifiedActionPolicyStrategy::continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(action);

    if (!m_d->changeSizeModifierActive && !m_d->anySamplerModifierActive) return;
    const bool altIsActive = event->modifiers() & Qt::AltModifier;

    QPointF imagePos = m_d->converter->documentToImage(event->point);
    m_d->lastImagePos = imagePos;

    continuePrimaryAction(imagePos, m_d->changeSizeModifierActive, altIsActive);
}

bool KisSimplifiedActionPolicyStrategy::endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(action);

    if (!m_d->changeSizeModifierActive && !m_d->anySamplerModifierActive) return false;

    QPointF imagePos = m_d->converter->documentToImage(event->point);
    m_d->lastImagePos = imagePos;

    return endPrimaryAction();
}

void KisSimplifiedActionPolicyStrategy::hoverActionCommon(const QPointF &pt)
{
    setTransformFunction(pt, m_d->anySamplerModifierActive && !m_d->sampleFromNodeModifierActive, m_d->changeSizeModifierActive, m_d->sampleFromNodeModifierActive);
}

