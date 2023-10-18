/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDumbAnimatedTransformMaskParamsHolder.h"

#include <KisDumbTransformMaskParams.h>
#include "KisChangeValueCommand.h"
#include "kis_pointer_utils.h"

KisDumbAnimatedTransformMaskParamsHolder::KisDumbAnimatedTransformMaskParamsHolder(KisDefaultBoundsBaseSP bounds)
    : m_defaultBounds(bounds)
    , m_params(new KisDumbTransformMaskParams())
{
}

KisDumbAnimatedTransformMaskParamsHolder::KisDumbAnimatedTransformMaskParamsHolder(const KisDumbAnimatedTransformMaskParamsHolder &rhs)
    : m_defaultBounds(rhs.m_defaultBounds)
    , m_params(rhs.m_params->clone())
{
}

bool KisDumbAnimatedTransformMaskParamsHolder::isAnimated() const
{
    return false;
}

KisKeyframeChannel *KisDumbAnimatedTransformMaskParamsHolder::requestKeyframeChannel(const QString &id) {
    Q_UNUSED(id);
    return nullptr;
}

KisKeyframeChannel *KisDumbAnimatedTransformMaskParamsHolder::getKeyframeChannel(const QString &id) const {
    Q_UNUSED(id);
    return nullptr;
}

KisTransformMaskParamsInterfaceSP KisDumbAnimatedTransformMaskParamsHolder::bakeIntoParams() const {
    return m_params->clone();
}

void KisDumbAnimatedTransformMaskParamsHolder::setParamsAtCurrentPosition(const KisTransformMaskParamsInterface *params, KUndo2Command *parentCommand) {
    makeChangeValueCommand<&KisDumbAnimatedTransformMaskParamsHolder::m_params>(
                this, params->clone(), parentCommand);
}

KisAnimatedTransformParamsHolderInterfaceSP KisDumbAnimatedTransformMaskParamsHolder::clone() const {
    return toQShared(new KisDumbAnimatedTransformMaskParamsHolder(*this));
}

void KisDumbAnimatedTransformMaskParamsHolder::setDefaultBounds(KisDefaultBoundsBaseSP bounds) {
    m_defaultBounds = bounds;
}

KisDefaultBoundsBaseSP KisDumbAnimatedTransformMaskParamsHolder::defaultBounds() const {
    return m_defaultBounds;
}

void KisDumbAnimatedTransformMaskParamsHolder::syncLodCache() {}
