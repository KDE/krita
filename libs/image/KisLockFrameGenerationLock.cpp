/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisLockFrameGenerationLock.h"

#include <kis_image_animation_interface.h>


KisLockFrameGenerationLockAdapter::KisLockFrameGenerationLockAdapter(KisImageAnimationInterface *interface)
    : m_interface(interface)
{
}

bool KisLockFrameGenerationLockAdapter::try_lock()
{
    return m_interface->tryLockFrameGeneration();
}

void KisLockFrameGenerationLockAdapter::lock()
{
    m_interface->lockFrameGeneration();
}

void KisLockFrameGenerationLockAdapter::unlock()
{
    m_interface->unlockFrameGeneration();
}
