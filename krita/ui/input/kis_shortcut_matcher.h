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

#ifndef __KIS_SHORTCUT_MATCHER_H
#define __KIS_SHORTCUT_MATCHER_H

#include "kis_abstract_shortcut.h"

#include <QList>
#include "kis_key_shortcut.h"

class QMouseEvent;
class QTabletEvent;

class KisStrokeShortcut;
class KisAbstractInputAction;

/**
 * The class that manages connections between shortcuts and actions.
 *
 * It processes input events and generates state transitions for the
 * actions basing on the data, represented by the shortcuts.
 *
 * \see KisStrokeShortcut
 * \see KisKeyShortcut
 */
class KRITAUI_EXPORT KisShortcutMatcher
{
public:
    KisShortcutMatcher();
    ~KisShortcutMatcher();

    void addShortcut(KisKeyShortcut *shortcut);
    void addShortcut(KisStrokeShortcut *shortcut);
    void addAction(KisAbstractInputAction *action);

    /**
     * Handles a key press event.
     *
     * \return whether the event has been handled successfully and
     * should be eaten by the events filter
     */
    bool keyPressed(Qt::Key key);

    /**
     * Handles a key release event.
     *
     * \return whether the event has been handled successfully and
     * should be eaten by the events filter
     */
    bool keyReleased(Qt::Key key);

    /**
     * Handles the mouse button press event
     *
     * \param button the button that has been pressed
     * \param event the event that caused this call
     *
     * \return whether the event has been handled successfully and
     * should be eaten by the events filter
     */
    bool buttonPressed(Qt::MouseButton button, QMouseEvent *event);

    /**
     * Handles the mouse button release event
     *
     * \param button the button that has been pressed
     * \param event the event that caused this call
     *
     * \return whether the event has been handled successfully and
     * should be eaten by the events filter
     */
    bool buttonReleased(Qt::MouseButton button, QMouseEvent *event);

    /**
     * Handles the mouse wheel event
     *
     * \return whether the event has been handled successfully and
     * should be eaten by the events filter
     */
    bool wheelEvent(KisKeyShortcut::WheelAction wheelAction);

    /**
     * Handles the mouse move event
     *
     * \param event the event that caused this call
     *
     * \return whether the event has been handled successfully and
     * should be eaten by the events filter
     */
    bool mouseMoved(QMouseEvent *event);

    /**
     * Resets the internal state of the matcher
     *
     * This should be done when the window has lost the focus for
     * some time, so that several events could be lost
     */
    void reset();

    /**
     * Disables the start of any actions.
     *
     * WARNING: the actions that has been started before this call
     * will *not* be ended. They will be ended in their usual way,
     * when the mouse button will be released.
     */
    void suppressAllActions(bool value);

private:
    friend class KisInputManagerTest;

    bool tryRunKeyShortcut(Qt::Key key);
    bool tryRunWheelShortcut(KisKeyShortcut::WheelAction wheelAction);
    template<typename T> bool tryRunKeyShortcutImpl(T param);

    void prepareReadyShortcuts();

    bool tryRunReadyShortcut(Qt::MouseButton button, QMouseEvent *event);
    void tryActivateReadyShortcut();
    bool tryEndRunningShortcut(Qt::MouseButton button, QMouseEvent *event);

private:
    class Private;
    Private * const m_d;
};

#endif /* __KIS_SHORTCUT_MATCHER_H */
