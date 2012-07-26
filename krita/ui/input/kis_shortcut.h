/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef KISSHORTCUT_H
#define KISSHORTCUT_H

#include <Qt>
#include <QList>

class QEvent;
class KisAbstractInputAction;

/**
 * \brief A combination of keys and buttons used for matching input.
 *
 * The Shortcut class manages a combination of keys and buttons and
 * the state of those buttons. It can be used to detect whether a certain
 * combination of inputs has been activated.
 */
class KisShortcut
{

public:
    /**
     * Describes how well recent input matches this shortcut.
     */
    enum MatchLevel {
        NoMatch, ///< No match at all.
        PartialMatch, ///< It may match, with additional input.
        CompleteMatch ///< Completely matches the input sent.
    };

    /**
     * States the mouse wheel can be in.
     */
    enum WheelState {
        WheelUndefined, ///< The state is unknown.
        WheelUp, ///< Mouse wheel moves up.
        WheelDown ///< Mouse wheel moves down.
    };

    /**
     * Constructor.
     */
    KisShortcut();
    /**
     * Destructor.
     */
    virtual ~KisShortcut();

    /**
     */
    int priority() const;
    /**
     * The action associated with this shortcut.
     */
    KisAbstractInputAction* action() const;
    /**
     * Set the action associated with this shortcut.
     */
    void setAction(KisAbstractInputAction *action);
    /**
     * The index of the shortcut.
     *
     * \see KisAbstractInputAction::begin()
     */
    int shortcutIndex() const;
    /**
     * Set the index of the shortcut.
     */
    void setShortcutIndex(int index);
    /**
     * Set the list of keys used by this shortcut.
     */
    void setKeys(const QList<Qt::Key> &keys);
    /**
     * Set the list of buttons used by this shortcut.
     */
    void setButtons(const QList<Qt::MouseButton> &buttons);
    /**
     * Set the wheel state to use for this shortcut.
     */
    void setWheel(WheelState state);
    /**
     * Returns how well this shortcut matches recent input.
     */
    MatchLevel matchLevel();
    /**
     * Try to match input to the keys and buttons used by
     * this shortcut.
     *
     * \param event An event to match.
     */
    void match(QEvent* event);
    /**
     * Clear all state of this shortcut.
     */
    void clear();

private:
    class Private;
    Private * const d;
};

#endif // KISSHORTCUT_H
