/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInputActionGroup.h"

KisInputActionGroupsMaskInterface::~KisInputActionGroupsMaskInterface() {
}

KisInputActionGroupsMaskGuard::KisInputActionGroupsMaskGuard(KisInputActionGroupsMaskInterface::SharedInterface sharedInterface, KisInputActionGroupsMask mask)
    : m_sharedInterface(sharedInterface),
      m_oldMask(sharedInterface->inputActionGroupsMask())
{
    m_sharedInterface->setInputActionGroupsMask(mask);
}

KisInputActionGroupsMaskGuard::~KisInputActionGroupsMaskGuard() {
    m_sharedInterface->setInputActionGroupsMask(m_oldMask);
}


