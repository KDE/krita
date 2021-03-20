/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SINGLE_ACTION_SHORTCUT_H
#define __KIS_SINGLE_ACTION_SHORTCUT_H

#include "kis_abstract_shortcut.h"

/**
 * This class represents a shortcut that executes a simple atomic
 * action. It can be initiated either by a keyboard hotkey or by
 * a mouse wheel rotation.
 */

class KRITAUI_EXPORT KisSingleActionShortcut : public KisAbstractShortcut
{
public:
    enum WheelAction {
        WheelUp, ///< Mouse wheel moves up.
        WheelDown, ///< Mouse wheel moves down.
        WheelLeft, ///< Mouse wheel moves left.
        WheelRight, ///< Mouse wheel moves right.
        WheelTrackpad, ///< A pan movement on a trackpad.
    };

    KisSingleActionShortcut(KisAbstractInputAction *action, int index);
    ~KisSingleActionShortcut() override;

    int priority() const override;

    void setKey(const QSet<Qt::Key> &modifiers, Qt::Key key);
    void setWheel(const QSet<Qt::Key> &modifiers, WheelAction wheelAction);

    bool match(const QSet<Qt::Key> &modifiers, Qt::Key key);
    bool match(const QSet<Qt::Key> &modifiers, WheelAction wheelAction);

private:
    class Private;
    Private * const m_d;
};

#endif /* __KIS_SINGLE_ACTION_SHORTCUT_H */
