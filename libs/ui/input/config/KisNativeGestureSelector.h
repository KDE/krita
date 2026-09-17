/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISNATIVEGESTURESELECTOR_H_
#define __KISNATIVEGESTURESELECTOR_H_

#include "input/kis_shortcut_configuration.h"
#include <kcombobox.h>

class KisNativeGestureSelector : public KComboBox
{
    Q_OBJECT
public:
    KisNativeGestureSelector(QWidget *parent);
    void setNativeGesture(KisShortcutConfiguration::NativeGestureAction gestureAction);
    KisShortcutConfiguration::NativeGestureAction nativeGesture();
};


#endif // __KISNATIVEGESTURESELECTOR_H_
