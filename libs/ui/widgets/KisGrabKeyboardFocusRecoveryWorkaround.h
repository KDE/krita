/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGRABKEYBOARDFOCUSRECOVERYWORKAROUND_H
#define KISGRABKEYBOARDFOCUSRECOVERYWORKAROUND_H

#include <QScopedPointer>

#include "kritaui_export.h"

/**
 * @brief Background: For some reason, when using a combination of
 *        'grabKeyboard' and 'releaseKeyboard' (e.g. the screen color sampling
 *        facility), the active window loses focus, or at least that is
 *        indicated by the change on the windows decorations (title bar, frame,
 *        etc.). Actually, the window seems to remain activated/focused
 *        regardless of the change on the window decorations. This is probably
 *        some bug on Qt/Plasma/X11 (a small and simple standalone test app
 *        gives the same results).
 * 
 *        Workaround: This class uses a dummy top-level widget that is shown and
 *        closed immediately. This activates the dummy widget and then the
 *        previous window is reactivated, effectively restoring the focused
 *        look.
 * 
 *        Usage: Use 'KisGrabKeyboardFocusRecoveryWorkaround::instance()->recoverFocus()
 *        when using a 'grabKeyboard'/'releaseKeyboard' if the top window seems
 *        to lose focus. Put that line right before 'releaseKeyboard' to prevent
 *        window decorations flickering.
 */
class KRITAUI_EXPORT KisGrabKeyboardFocusRecoveryWorkaround
{
public:
    static KisGrabKeyboardFocusRecoveryWorkaround* instance();

    void recoverFocus();

private:
    class Private;
    QScopedPointer<Private> m_d;
    
    KisGrabKeyboardFocusRecoveryWorkaround();
    Q_DISABLE_COPY(KisGrabKeyboardFocusRecoveryWorkaround)
};

#endif
