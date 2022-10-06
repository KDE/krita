/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QApplication>

#include "KisKeyboardModifierWatcher.h"

KisKeyboardModifierWatcher::KisKeyboardModifierWatcher(bool sendIndividualSignals, int watchInterval)
    : m_modifiers(Qt::NoModifier)
    , m_sendIndividualSignals(sendIndividualSignals)
{
    m_timer.setInterval(watchInterval);
    connect(&m_timer, SIGNAL(timeout()), SLOT(slotTimerTimeout()));
}

KisKeyboardModifierWatcher::~KisKeyboardModifierWatcher()
{}

void KisKeyboardModifierWatcher::startWatching()
{
    slotTimerTimeout();
    m_timer.start();
}

void KisKeyboardModifierWatcher::stopWatching()
{
    m_timer.stop();
}

Qt::KeyboardModifiers KisKeyboardModifierWatcher::modifiers() const
{
    return m_modifiers;
}

bool KisKeyboardModifierWatcher::isModifierPressed(Qt::KeyboardModifier modifier) const
{
    return m_modifiers.testFlag(modifier);
}

void KisKeyboardModifierWatcher::slotTimerTimeout()
{
    // Take into account only common modifiers
    const Qt::KeyboardModifiers previousModifiers = m_modifiers;
    m_modifiers = qApp->queryKeyboardModifiers() & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
    if (previousModifiers != m_modifiers) {
        if (m_sendIndividualSignals) {
            if (previousModifiers.testFlag(Qt::ShiftModifier) !=
                m_modifiers.testFlag(Qt::ShiftModifier)) {
                emit modifierChanged(Qt::ShiftModifier, m_modifiers.testFlag(Qt::ShiftModifier));

            }
            if (previousModifiers.testFlag(Qt::ControlModifier) !=
                m_modifiers.testFlag(Qt::ControlModifier)) {
                emit modifierChanged(Qt::ControlModifier, m_modifiers.testFlag(Qt::ControlModifier));
            }
            if (previousModifiers.testFlag(Qt::AltModifier) !=
                m_modifiers.testFlag(Qt::AltModifier)) {
                emit modifierChanged(Qt::AltModifier, m_modifiers.testFlag(Qt::AltModifier));
            }
            if (previousModifiers.testFlag(Qt::MetaModifier) !=
                m_modifiers.testFlag(Qt::MetaModifier)) {
                emit modifierChanged(Qt::MetaModifier, m_modifiers.testFlag(Qt::MetaModifier));
            }
            if (previousModifiers.testFlag(Qt::KeypadModifier) !=
                m_modifiers.testFlag(Qt::KeypadModifier)) {
                emit modifierChanged(Qt::KeypadModifier, m_modifiers.testFlag(Qt::KeypadModifier));
            }
            if (previousModifiers.testFlag(Qt::GroupSwitchModifier) !=
                m_modifiers.testFlag(Qt::GroupSwitchModifier)) {
                emit modifierChanged(Qt::GroupSwitchModifier, m_modifiers.testFlag(Qt::GroupSwitchModifier));
            }
        }

        emit modifiersChanged(m_modifiers);
    }
}
