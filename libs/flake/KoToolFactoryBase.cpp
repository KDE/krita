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

#include "KoToolFactoryBase.h"

class KoToolFactoryBase::Private
{
public:
    Private(const QString &i)
            : priority(100),
            id(i)
    {
    }
    int priority;
    QString toolType;
    QString tooltip;
    QString activationId;
    QString iconName;
    const QString id;
    KShortcut shortcut;
};


KoToolFactoryBase::KoToolFactoryBase(const QString &id)
        : d(new Private(id))
{
}

KoToolFactoryBase::~KoToolFactoryBase()
{
    delete d;
}

QString KoToolFactoryBase::id() const
{
    return d->id;
}

int KoToolFactoryBase::priority() const
{
    return d->priority;
}

QString KoToolFactoryBase::toolType() const
{
    return d->toolType;
}

QString KoToolFactoryBase::toolTip() const
{
    return d->tooltip;
}

QString KoToolFactoryBase::iconName() const
{
    return d->iconName;
}

QString KoToolFactoryBase::activationShapeId() const
{
    return d->activationId;
}

KShortcut KoToolFactoryBase::shortcut() const
{
    return d->shortcut;
}

void KoToolFactoryBase::setActivationShapeId(const QString &activationShapeId)
{
    d->activationId = activationShapeId;
}

void KoToolFactoryBase::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KoToolFactoryBase::setToolType(const QString & toolType)
{
    d->toolType = toolType;
}

void KoToolFactoryBase::setIconName(const char *iconName)
{
    d->iconName = QLatin1String(iconName);
}

void KoToolFactoryBase::setIconName(const QString &iconName)
{
    d->iconName = iconName;
}

void KoToolFactoryBase::setPriority(int newPriority)
{
    d->priority = newPriority;
}

void KoToolFactoryBase::setShortcut(const KShortcut & shortcut)
{
    d->shortcut = shortcut;
}

bool KoToolFactoryBase::canCreateTool(KoCanvasBase *) const
{
    return true;
}
