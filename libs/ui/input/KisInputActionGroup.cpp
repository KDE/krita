/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInputActionGroup.h"

KisInputActionGroupsMaskInterface::~KisInputActionGroupsMaskInterface() {}

KisInputActionGroupsMaskGuard::KisInputActionGroupsMaskGuard(KisInputActionGroupsMaskInterface *object, KisInputActionGroupsMask mask)
    : m_object(object),
      m_oldMask(object->inputActionGroupsMask())
{
    m_object->setInputActionGroupsMask(mask);
}

KisInputActionGroupsMaskGuard::~KisInputActionGroupsMaskGuard() {
    m_object->setInputActionGroupsMask(m_oldMask);
}


