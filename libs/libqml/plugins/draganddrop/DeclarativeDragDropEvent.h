/*
    SPDX-FileCopyrightText: 2010 BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    SPDX-License-Identifier: MIT
*/

#ifndef DECLARATIVEDRAGDROPEVENT_H
#define DECLARATIVEDRAGDROPEVENT_H

#include <QObject>
#include "DeclarativeDropArea.h"

class DeclarativeMimeData;

class DeclarativeDragDropEvent : public QObject
{
    Q_OBJECT

    /**
     * The mouse X position of the event relative to the DropArea that is receiving the event.
     */
    Q_PROPERTY(int x READ x)

    /**
     * The mouse Y position of the event relative to the DropArea that is receiving the event.
     */
    Q_PROPERTY(int y READ y)

    /**
     * The pressed mouse buttons.
     * A combination of:
     *  Qt.NoButton    The button state does not refer to any button (see QMouseEvent::button()).
     *  Qt.LeftButton    The left button is pressed, or an event refers to the left button. (The left button may be the right button on left-handed mice.)
     *  Qt.RightButton    The right button.
     *  Qt.MidButton    The middle button.
     *  Qt.MiddleButton  MidButton  The middle button.
     *  Qt.XButton1    The first X button.
     *  Qt.XButton2    The second X button.
     */
    Q_PROPERTY(int buttons READ buttons)

    /**
     * Pressed keyboard modifiers, a combination of:
     *  Qt.NoModifier    No modifier key is pressed.
     *  Qt.ShiftModifier    A Shift key on the keyboard is pressed.
     *  Qt.ControlModifier    A Ctrl key on the keyboard is pressed.
     *  Qt.AltModifier    An Alt key on the keyboard is pressed.
     *  Qt.MetaModifier    A Meta key on the keyboard is pressed.
     *  Qt.KeypadModifier    A keypad button is pressed.
     *  Qt.GroupSwitchModifier    X11 only. A Mode_switch key on the keyboard is pressed.
     */
    Q_PROPERTY(int modifiers READ modifiers)

    /**
     * The mime data of this operation
     * @see DeclarativeMimeData
     */
    Q_PROPERTY(DeclarativeMimeData* mimeData READ mimeData)

    /**
     * The possible different kind of action that can be done in the drop, is a combination of:
     *  Qt.CopyAction  0x1  Copy the data to the target.
     *  Qt.MoveAction  0x2  Move the data from the source to the target.
     *  Qt.LinkAction  0x4  Create a link from the source to the target.
     *  Qt.ActionMask  0xff   
     *  Qt.IgnoreAction  0x0  Ignore the action (do nothing with the data).
     *  Qt.TargetMoveAction  0x8002  On Windows, this value is used when the ownership of the D&D data should be taken over by the target application, i.e., the source application should not delete the data.
     *  On X11 this value is used to do a move.
     *  TargetMoveAction is not used on the Mac.
     */
    Q_PROPERTY(Qt::DropActions possibleActions READ possibleActions)

    /**
     * Default action
     * @see possibleActions
     */
    Q_PROPERTY(Qt::DropAction proposedAction READ proposedAction)

public:

    DeclarativeDragDropEvent(QDropEvent* e, DeclarativeDropArea* parent = 0);
    DeclarativeDragDropEvent(QDragLeaveEvent* e, DeclarativeDropArea* parent = 0);

    int x() const { return m_x; }
    int y() const { return m_y; }
    int buttons() const { return m_buttons; }
    int modifiers() const { return m_modifiers; }
    DeclarativeMimeData* mimeData();
    Qt::DropAction proposedAction() const { return m_event->proposedAction(); }
    Qt::DropActions possibleActions() const { return m_event->possibleActions(); }

public Q_SLOTS:
    void accept(int action);

private:
    int m_x;
    int m_y;
    Qt::MouseButtons m_buttons;
    Qt::KeyboardModifiers m_modifiers;
    DeclarativeMimeData* m_data;
    QDropEvent* m_event;
};

#endif // DECLARATIVEDRAGDROPEVENT_H

