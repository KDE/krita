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

#ifndef KIS_ABSTRACT_INPUT_ACTION_H
#define KIS_ABSTRACT_INPUT_ACTION_H

#include <QHash>
#include "krita_export.h"

class QPointF;
class QEvent;
class KisInputManager;

/**
 * \brief Abstract base class for input actions.
 *
 * Input actions represent actions to be performed when interacting
 * with the canvas. They are managed by KisInputManager and activated
 * when KisKeyShortcut or KisStrokeShortcut detects it matches a certain
 * set of inputs.
 *
 * The begin() method uses an index for the type of behaviour to activate.
 * This index can be used to trigger behaviour when different events occur.
 *
 * The events can be of two types:
 * 1) Key events. The input manager calls begin() and end() sequentially
 *    with an \p index parameter to begin() representing the type of
 *    action that should be performed. The \p event parameter of both
 *    calls in null.
 * 2) Stroke events. The input manager calls begin() and end() on the
 *    corresponding mouse down and up events. The \p event parameter
 *    will be of QMouseEvent type, representing the event happened.
 *    All the mouse move events between begin() and end() will be
 *    redirected to the inputEvent() method.
 *
 *    You can fetch the QTabletEvent data for the current mouse event
 *    with inputManager()->lastTabletEvent().
 */
class KRITAUI_EXPORT KisAbstractInputAction
{
public:
    /**
     * Constructor.
     *
     * \param manager The InputManager this action belongs to.
     */
    explicit KisAbstractInputAction();
    /**
     * Destructor.
     */
    virtual ~KisAbstractInputAction();

    /**
     * The method is called when the action is yet to be started,
     * that is, e.g. the user has pressed all the modifiers for the
     * action but hasn't started painting yet. This method is a right
     * place to show the user what he is going to do, e.g. change the
     * cursor.
     */
    virtual void activate();

    /**
     * The method is called when the action is not a candidate for
     * the starting anymore. The action should revert everything that
     * was done in activate() method.
     *
     * \see activate()
     */
    virtual void deactivate();

    /**
     * Begin the action.
     *
     * \param shortcut The index of the behaviour to trigger.
     * \param event The mouse event that has triggered this action.
     *              Is null for keyboard-activated actions.
     */
    virtual void begin(int shortcut, QEvent *event);
    /**
     * End the action.
     * \param event The mouse event that has finished this action.
     *              Is null for keyboard-activated actions.
     */
    virtual void end(QEvent *event);
    /**
     * Process an input event.
     *
     * By default handles MouseMove events and passes the data to
     * a convenience mouseMoved() method
     *
     * \param event An event to process.
     */
    virtual void inputEvent(QEvent* event);

    /**
     * On some platforms (Windows in particular), tablet and mouse
     * events generate different flows of messages. The amount of
     * tablet events may be hard to process for some actions, so it is
     * false by default. On Linux platform the flows are exactly the
     * same so there is no difference between tablet and mouse events.
     */
    virtual bool supportsHiResInputEvents() const;

    /**
     * The indexes of shortcut behaviours available.
     */
    virtual QHash<QString, int> shortcutIndexes() const;
    /**
     * The name of this action.
     */
    virtual QString name() const;
    /**
     * A short description of this action.
     */
    virtual QString description() const;

protected:
    /**
     * The input manager this action belongs to.
     */
    KisInputManager *inputManager() const;
    /**
     * Set the name of this action.
     *
     * \param name The new name.
     */
    void setName(const QString &name);
    /**
     * Set the description of this action.
     *
     * \param description The new description.
     */
    void setDescription(const QString &description);
    /**
     * Set the available indexes of shortcut behaviours.
     *
     * \param indexes The new indexes.
     */
    void setShortcutIndexes(const QHash<QString, int> &indexes);

    /**
     * Convenience method for handling the mouse moves. It is
     * called by the default implementation of inputEvent
     */
    virtual void mouseMoved(const QPointF &lastPos, const QPointF &pos);

private:
    friend class KisInputManager;
    static void setInputManager(KisInputManager *manager);

    class Private;
    Private * const d;
};

#endif // KIS_ABSTRACT_INPUT_ACTION_H
