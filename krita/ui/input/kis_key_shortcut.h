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

#ifndef __KIS_KEY_SHORTCUT_H
#define __KIS_KEY_SHORTCUT_H

#include "kis_abstract_shortcut.h"

/**
 * This class represents a shortcut that is started with simple
 * key presses only, that is a simple keyboard hotkey or a mouse
 * wheel rotation by one delta.
 */

class KRITAUI_EXPORT KisKeyShortcut : public KisAbstractShortcut
{
public:
    enum WheelAction {
        WheelUp, ///< Mouse wheel moves up.
        WheelDown ///< Mouse wheel moves down.
    };

    KisKeyShortcut(KisAbstractInputAction *action, int index);
    ~KisKeyShortcut();

    int priority() const;

    void setKey(const QList<Qt::Key> &modifiers, Qt::Key key);
    void setWheel(const QList<Qt::Key> &modifiers, WheelAction wheelAction);

    bool matchKey(const QList<Qt::Key> &modifiers, Qt::Key key);
    bool matchKey(const QList<Qt::Key> &modifiers, WheelAction wheelAction);

private:
    class Private;
    Private * const m_d;
};

#endif /* __KIS_KEY_SHORTCUT_H */
