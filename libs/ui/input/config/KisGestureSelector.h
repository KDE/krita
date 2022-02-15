/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISGESTURESELECTOR_H_
#define __KISGESTURESELECTOR_H_

#include "input/kis_shortcut_configuration.h"
#include <kcombobox.h>

class KisGestureSelector : public KComboBox
{
    Q_OBJECT
public:
    KisGestureSelector(QWidget *parent);
    void setGesture(KisShortcutConfiguration::GestureAction gestureAction);
    KisShortcutConfiguration::GestureAction gesture();
};


#endif // __KISGESTURESELECTOR_H_
