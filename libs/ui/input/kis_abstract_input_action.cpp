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

#include "kis_abstract_input_action.h"

#include <QPointF>
#include <QMouseEvent>
#include <klocalizedstring.h>
#include <kis_debug.h>

class Q_DECL_HIDDEN KisAbstractInputAction::Private
{
public:
    QString id;
    QString name;
    QString description;
    QHash<QString, int> indexes;

    QPointF lastCursorPosition;

    static KisInputManager *inputManager;
};

KisInputManager *KisAbstractInputAction::Private::inputManager = 0;

KisAbstractInputAction::KisAbstractInputAction(const QString &id)
    : d(new Private)
{
    d->id = id;
    d->indexes.insert(i18n("Activate"), 0);
}

KisAbstractInputAction::~KisAbstractInputAction()
{
    delete d;
}

void KisAbstractInputAction::activate(int shortcut)
{
    Q_UNUSED(shortcut);
}

void KisAbstractInputAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
}

void KisAbstractInputAction::begin(int shortcut, QEvent *event)
{
    Q_UNUSED(shortcut);

    if (event) {
        d->lastCursorPosition = eventPosF(event);
    }
}

void KisAbstractInputAction::inputEvent(QEvent *event)
{
    if (event) {
        QPointF newPosition = eventPosF(event);
        cursorMoved(d->lastCursorPosition, newPosition);
        d->lastCursorPosition = newPosition;
    }
}

void KisAbstractInputAction::end(QEvent *event)
{
    Q_UNUSED(event);
}

void KisAbstractInputAction::cursorMoved(const QPointF &lastPos, const QPointF &pos)
{
    Q_UNUSED(lastPos);
    Q_UNUSED(pos);
}

bool KisAbstractInputAction::supportsHiResInputEvents() const
{
    return false;
}

KisInputActionGroup KisAbstractInputAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ModifyingActionGroup;
}

KisInputManager* KisAbstractInputAction::inputManager() const
{
    return Private::inputManager;
}

QString KisAbstractInputAction::name() const
{
    return d->name;
}

QString KisAbstractInputAction::description() const
{
    return d->description;
}

int KisAbstractInputAction::priority() const
{
    return 0;
}

bool KisAbstractInputAction::canIgnoreModifiers() const
{
    return false;
}

QHash< QString, int > KisAbstractInputAction::shortcutIndexes() const
{
    return d->indexes;
}

QString KisAbstractInputAction::id() const
{
    return d->id;
}

void KisAbstractInputAction::setName(const QString &name)
{
    d->name = name;
}

void KisAbstractInputAction::setDescription(const QString &description)
{
    d->description = description;
}

void KisAbstractInputAction::setShortcutIndexes(const QHash< QString, int > &indexes)
{
    d->indexes = indexes;
}

void KisAbstractInputAction::setInputManager(KisInputManager *manager)
{
    Private::inputManager = manager;
}

bool KisAbstractInputAction::isShortcutRequired(int shortcut) const
{
    Q_UNUSED(shortcut);
    return false;
}

QPoint KisAbstractInputAction::eventPos(const QEvent *event)
{
    if(!event) {
        return QPoint();
    }

    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonRelease:
        return static_cast<const QMouseEvent*>(event)->pos();

    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
        return static_cast<const QTabletEvent*>(event)->pos();

    case QEvent::Wheel:
        return static_cast<const QWheelEvent*>(event)->pos();

    case QEvent::NativeGesture:
        return static_cast<const QNativeGestureEvent*>(event)->pos();

    default:
        warnInput << "KisAbstractInputAction" << d->name << "tried to process event data from an unhandled event type" << event->type();
        return QPoint();
    }
}

QPointF KisAbstractInputAction::eventPosF(const QEvent *event) {

    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonRelease:
        return static_cast<const QMouseEvent*>(event)->localPos();

    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
        return static_cast<const QTabletEvent*>(event)->posF();

    case QEvent::Wheel:
        return static_cast<const QWheelEvent*>(event)->posF();

    case QEvent::NativeGesture:
        return QPointF(static_cast<const QNativeGestureEvent*>(event)->pos());

    default:
        warnInput << "KisAbstractInputAction" << d->name << "tried to process event data from an unhandled event type" << event->type();
        return QPointF();
    }
}

bool KisAbstractInputAction::isAvailable() const
{
    return true;
}
