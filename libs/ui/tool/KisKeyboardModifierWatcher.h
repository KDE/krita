/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISKEYBOARDMODIFIERWATCHER_H
#define KISKEYBOARDMODIFIERWATCHER_H

#include <QObject>
#include <QTimer>

#include "kritaui_export.h"

class KRITAUI_EXPORT KisKeyboardModifierWatcher : public QObject
{
    Q_OBJECT

public:
    KisKeyboardModifierWatcher(bool sendIndividualSignals = true, int watchInterval = 100);
    ~KisKeyboardModifierWatcher() override;

    void startWatching();
    void stopWatching();
    Qt::KeyboardModifiers modifiers() const;
    bool isModifierPressed(Qt::KeyboardModifier modifier) const;

Q_SIGNALS:
    void modifiersChanged(Qt::KeyboardModifiers modifiers);
    void modifierChanged(Qt::KeyboardModifier modifier, bool isPressed);

private:
    Qt::KeyboardModifiers m_modifiers;
    const bool m_sendIndividualSignals;
    QTimer m_timer;

private Q_SLOTS:
    void slotTimerTimeout();
};

#endif
