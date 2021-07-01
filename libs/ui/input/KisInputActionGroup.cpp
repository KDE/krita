/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInputActionGroup.h"

KisInputActionGroupsMaskInterface::~KisInputActionGroupsMaskInterface() {
    if(m_sharedReference) {
        // unregister ourselves in case a guard is pointing at us (and avoid them trying to update already-deleted `this`)
        m_sharedReference->m_ref = nullptr;
    }
}
KisInputActionGroupsMaskInterface::SharedReference KisInputActionGroupsMaskInterface::getSharedReference() {
    if(!m_sharedReference) {
        m_sharedReference = SharedReference(new Reference);
        m_sharedReference->m_ref = this;
    }
    return m_sharedReference;
}

KisInputActionGroupsMaskGuard::KisInputActionGroupsMaskGuard(KisInputActionGroupsMaskInterface::SharedReference interfaceReference, KisInputActionGroupsMask mask)
    : m_interfaceReference(interfaceReference),
      m_oldMask(interfaceReference->m_ref->inputActionGroupsMask())
{
    interfaceReference->m_ref->setInputActionGroupsMask(mask);
}

KisInputActionGroupsMaskGuard::~KisInputActionGroupsMaskGuard() {
    if(m_interfaceReference->m_ref) // only update if view hasn't been deleted
        m_interfaceReference->m_ref->setInputActionGroupsMask(m_oldMask);
}


