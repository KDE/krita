/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisNativeGestureSelector.h"

KisNativeGestureSelector::KisNativeGestureSelector(QWidget *parent)
    : KComboBox(parent)
{
    insertItem(0, KisShortcutConfiguration::nativeGestureToText(KisShortcutConfiguration::PinchGesture), static_cast<int>(KisShortcutConfiguration::PinchGesture));
    insertItem(1, KisShortcutConfiguration::nativeGestureToText(KisShortcutConfiguration::SmartZoomGesture), static_cast<int>(KisShortcutConfiguration::SmartZoomGesture));
    insertItem(2, KisShortcutConfiguration::nativeGestureToText(KisShortcutConfiguration::TouchpadScroll), static_cast<int>(KisShortcutConfiguration::TouchpadScroll));
}

void KisNativeGestureSelector::setNativeGesture(KisShortcutConfiguration::NativeGestureAction gestureAction)
{
    const int index = findData(static_cast<int>(gestureAction));
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

KisShortcutConfiguration::NativeGestureAction KisNativeGestureSelector::nativeGesture()
{
    return static_cast<KisShortcutConfiguration::NativeGestureAction>(currentData().toInt());
}
