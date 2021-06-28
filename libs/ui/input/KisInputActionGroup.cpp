/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInputActionGroup.h"

KisInputActionGroupsMaskInterface::~KisInputActionGroupsMaskInterface() {
    Q_FOREACH(KisInputActionGroupsMaskGuard *maskGuard, mRegisteredKisInputActionGroupsMaskGuards){
        maskGuard->maskingInterfaceWasDeleted();
    }
}
void KisInputActionGroupsMaskInterface::registerInputActionGroupsMaskGuard(KisInputActionGroupsMaskGuard *maskGuard) {
    mRegisteredKisInputActionGroupsMaskGuards << maskGuard;
}

KisInputActionGroupsMaskGuard::KisInputActionGroupsMaskGuard(KisInputActionGroupsMaskInterface *object, KisInputActionGroupsMask mask)
    : m_object(object),
      m_oldMask(object->inputActionGroupsMask())
{
    m_object->registerInputActionGroupsMaskGuard(this);
    m_object->setInputActionGroupsMask(mask);
}

void KisInputActionGroupsMaskGuard::maskingInterfaceWasDeleted() {
    m_object = nullptr;
}
KisInputActionGroupsMaskGuard::~KisInputActionGroupsMaskGuard() {
    if(m_object)
        m_object->setInputActionGroupsMask(m_oldMask);
}


