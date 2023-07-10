/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBlockBackgroundFrameGenerationLock.h"

#include <kis_image_animation_interface.h>

KisBlockBackgroundFrameGenerationLockAdapter::KisBlockBackgroundFrameGenerationLockAdapter(KisImageAnimationInterface *interface)
    : m_interface(interface)
{
}

void KisBlockBackgroundFrameGenerationLockAdapter::lock()
{
    m_interface->blockBackgroundFrameGeneration();
}

void KisBlockBackgroundFrameGenerationLockAdapter::unlock()
{
    m_interface->unblockBackgroundFrameGeneration();
}
