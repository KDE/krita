/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISTOUCHGESTUREACTION_H_
#define __KISTOUCHGESTUREACTION_H_

#include "kis_abstract_input_action.h"

class KisTouchGestureAction : public KisAbstractInputAction
{
public:
    enum Shortcut {
        UndoActionShortcut,
        RedoActionShortcut,
        ToggleCanvasOnlyShortcut,
        ToggleEraserMode,
    };

    KisTouchGestureAction();

    void begin(int shortcut, QEvent *event) override;
    void end(QEvent *event) override;

    int priority() const override;

private:
    int m_shortcut{-1};
};

#endif // __KISTOUCHGESTUREACTION_H_
