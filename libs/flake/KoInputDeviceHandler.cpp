/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoInputDeviceHandler.h"

class Q_DECL_HIDDEN KoInputDeviceHandler::Private
{
public:
    Private(const QString &devId)
            : id(devId) {
    }
    const QString id;
};

KoInputDeviceHandler::KoInputDeviceHandler(QObject * parent, const QString &id)
        : QObject(parent), d(new Private(id))
{
}

KoInputDeviceHandler::~KoInputDeviceHandler()
{
    delete d;
}

QString KoInputDeviceHandler::id() const
{
    return d->id;
}
