/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisNativeGestureSelector.h"

KisNativeGestureSelector::KisNativeGestureSelector(QWidget *parent)
    : KComboBox(parent)
{
    QStringList gestures;
    for (int i = 1; i < KisShortcutConfiguration::MaxNativeGesture; i++) {
        gestures << KisShortcutConfiguration::nativeGestureToText(static_cast<KisShortcutConfiguration::NativeGestureAction>(i));
    }
    addItems(gestures);
}

void KisNativeGestureSelector::setNativeGesture(KisShortcutConfiguration::NativeGestureAction gestureAction)
{
    setCurrentIndex(gestureAction - 1);
}

KisShortcutConfiguration::NativeGestureAction KisNativeGestureSelector::nativeGesture()
{
    return static_cast<KisShortcutConfiguration::NativeGestureAction>(currentIndex() + 1);
}
