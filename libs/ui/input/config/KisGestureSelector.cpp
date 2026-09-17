/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisGestureSelector.h"

KisGestureSelector::KisGestureSelector(QWidget *parent)
    : KComboBox(parent)
{
    QStringList gestures;
    for (int i = 1; i < KisShortcutConfiguration::MaxGesture; i++) {
        gestures << KisShortcutConfiguration::touchGestureToText(static_cast<KisShortcutConfiguration::TouchGestureAction>(i));
    }
    addItems(gestures);
}

void KisGestureSelector::setTouchGesture(KisShortcutConfiguration::TouchGestureAction gestureAction)
{
    setCurrentIndex(gestureAction - 1);
}

KisShortcutConfiguration::TouchGestureAction KisGestureSelector::touchGesture()
{
    return static_cast<KisShortcutConfiguration::TouchGestureAction>(currentIndex() + 1);
}
