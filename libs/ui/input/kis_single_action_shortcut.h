/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
