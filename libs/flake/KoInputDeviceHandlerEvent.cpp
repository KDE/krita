/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoInputDeviceHandlerEvent.h"
#include "KoPointerEvent.h"
#include <QApplication>

class Q_DECL_HIDDEN KoInputDeviceHandlerEvent::Private
{
public:
    Private()
            : button(Qt::NoButton), buttons(Qt::NoButton) {}

    KoInputDeviceHandlerEvent::Type type;
    Qt::MouseButton button;
    Qt::MouseButtons buttons;
};

KoInputDeviceHandlerEvent::KoInputDeviceHandlerEvent(Type type)
        : QInputEvent(static_cast<QEvent::Type>(type)), m_event(0), d(new Private())
{
    modState = QApplication::keyboardModifiers();
}

KoInputDeviceHandlerEvent::~KoInputDeviceHandlerEvent()
{
    delete m_event;
    delete d;
}

Qt::MouseButton KoInputDeviceHandlerEvent::button() const
{
    return d->button;
}

Qt::MouseButtons KoInputDeviceHandlerEvent::buttons() const
{
    return d->buttons;
}

void KoInputDeviceHandlerEvent::setButton(Qt::MouseButton b)
{
    d->button = b;
}

void KoInputDeviceHandlerEvent::setButtons(Qt::MouseButtons b)
{
    d->buttons = b;
}
