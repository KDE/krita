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

#include "kis_shortcut.h"

#include "kis_abstract_input_action.h"
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>

class KisShortcut::Private
{
public:
    Private() : wheelState(WheelUndefined), currentWheelState(WheelUndefined), action(0), shortcutIndex(0) { }
    QList<Qt::Key> keys;
    QList<Qt::Key> keyState;
    QList<Qt::MouseButton> buttons;
    QList<Qt::MouseButton> buttonState;
    WheelState wheelState;
    WheelState currentWheelState;

    KisAbstractInputAction *action;
    int shortcutIndex;
};

KisShortcut::KisShortcut() : d(new Private)
{
}

KisShortcut::~KisShortcut()
{
    delete d;
}

int KisShortcut::priority() const
{
    return d->keys.count() * 2 + d->buttons.count();
}

KisAbstractInputAction* KisShortcut::action() const
{
    return d->action;
}

void KisShortcut::setAction(KisAbstractInputAction* action)
{
    d->action = action;
}

int KisShortcut::shortcutIndex() const
{
    return d->shortcutIndex;
}

void KisShortcut::setShortcutIndex(int index)
{
    d->shortcutIndex = index;
}

void KisShortcut::setButtons(const QList<Qt::MouseButton> &buttons)
{
    d->buttons = buttons;
    d->buttonState.clear();
}

void KisShortcut::setKeys(const QList< Qt::Key >& keys)
{
    d->keys = keys;
    d->keyState.clear();
}

void KisShortcut::setWheel(KisShortcut::WheelState state)
{
    d->wheelState = state;
}

KisShortcut::MatchLevel KisShortcut::matchLevel()
{
    if (d->keys.count() == d->keyState.count() && d->buttons.count() == d->buttonState.count() && (d->wheelState == WheelUndefined || d->currentWheelState == d->wheelState)) {
        return CompleteMatch;
    } else if (d->keyState.count() > 0 || d->buttonState.count() > 0) {
        return PartialMatch;
    }

    return NoMatch;
}

void KisShortcut::match(QEvent* event)
{
    switch (event->type()) {
        case QEvent::KeyPress: {
            QKeyEvent *kevent = static_cast<QKeyEvent*>(event);
            Qt::Key key = static_cast<Qt::Key>(kevent->key());
            if (d->keys.contains(key) && !d->keyState.contains(key)) {
                d->keyState.append(key);
            }
            break;
        }
        case QEvent::KeyRelease: {
            QKeyEvent *kevent = static_cast<QKeyEvent*>(event);
            Qt::Key key = static_cast<Qt::Key>(kevent->key());
            if (d->keyState.contains(key)) {
                d->keyState.removeOne(key);
            }
            break;
        }
        case QEvent::MouseButtonPress: {
            Qt::MouseButton button = static_cast<QMouseEvent*>(event)->button();
            if (d->buttons.contains(button) && !d->buttonState.contains(button)) {
                d->buttonState.append(button);
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            Qt::MouseButton button = static_cast<QMouseEvent*>(event)->button();
            if (d->buttonState.contains(button)) {
                d->buttonState.removeOne(button);
            }
            break;
        }
        case QEvent::MouseButtonDblClick: {
            Qt::MouseButton button = static_cast<QMouseEvent*>(event)->button();
            if (d->buttons.contains(button) && !d->buttonState.contains(button)) {
                d->buttonState.append(button);
            }
            break;
        }
        case QEvent::Wheel: {
            QWheelEvent *wevent = static_cast<QWheelEvent*>(event);
            if (wevent->delta() > 0) {
                d->currentWheelState = WheelUp;
            } else {
                d->currentWheelState = WheelDown;
            }
            break;
        }
        default:
            break;
    }
}

void KisShortcut::clear()
{
    d->buttonState.clear();
    d->keyState.clear();
    d->currentWheelState = WheelUndefined;
}
