/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoToolFactory.h"

class KoToolFactory::Private
{
public:
    Private(const QString &i)
            : priority(100),
            inputDeviceAgnostic(true),
            id(i)
    {
    }
    int priority;
    bool inputDeviceAgnostic;
    QString toolType;
    QString tooltip;
    QString activationId;
    QString icon;
    const QString id;
    KShortcut shortcut;
};


KoToolFactory::KoToolFactory(QObject *parent, const QString &id)
        : QObject(parent),
        d(new Private(id))
{
}

KoToolFactory::~KoToolFactory()
{
    delete d;
}

QString KoToolFactory::id() const
{
    return d->id;
}

int KoToolFactory::priority() const
{
    return d->priority;
}

QString KoToolFactory::toolType() const
{
    return d->toolType;
}

QString KoToolFactory::toolTip() const
{
    return d->tooltip;
}

QString KoToolFactory::icon() const
{
    return d->icon;
}

QString KoToolFactory::activationShapeId() const
{
    return d->activationId;
}

KShortcut KoToolFactory::shortcut() const
{
    return d->shortcut;
}

void KoToolFactory::setActivationShapeId(const QString &activationShapeId)
{
    d->activationId = activationShapeId;
}

void KoToolFactory::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KoToolFactory::setToolType(const QString & toolType)
{
    d->toolType = toolType;
}

void KoToolFactory::setIcon(const QString & icon)
{
    d->icon = icon;
}

void KoToolFactory::setPriority(int newPriority)
{
    d->priority = newPriority;
}

void KoToolFactory::setShortcut(const KShortcut & shortcut)
{
    d->shortcut = shortcut;
}

void KoToolFactory::setInputDeviceAgnostic(bool agnostic)
{
    d->inputDeviceAgnostic = agnostic;
}

bool KoToolFactory::inputDeviceAgnostic() const
{
    return d->inputDeviceAgnostic;
}

bool KoToolFactory::canCreateTool(KoCanvasBase *) const
{
    return true;
}

#include <KoToolFactory.moc>
