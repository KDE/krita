/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_input_profile.h"

#include <QStringList>
#include <QMultiHash>

#include "kis_abstract_input_action.h"
#include "kis_shortcut_configuration.h"

class KisInputProfile::Private
{
public:
    Private() { }
    ~Private()
    {
        qDeleteAll(shortcuts);
    }

    QString name;
    QMultiHash<KisAbstractInputAction *, KisShortcutConfiguration *> shortcuts;
};

KisInputProfile::KisInputProfile(QObject *parent)
    : QObject(parent), d(new Private())
{

}

KisInputProfile::~KisInputProfile()
{
    delete d;
}

QString KisInputProfile::name() const
{
    return d->name;
}
void KisInputProfile::setName(const QString &name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged();
    }
}

QList< KisShortcutConfiguration * > KisInputProfile::allShortcuts() const
{
    return d->shortcuts.values();
}

QList< KisShortcutConfiguration * > KisInputProfile::shortcutsForAction(KisAbstractInputAction *action) const
{
    if (d->shortcuts.contains(action)) {
        return d->shortcuts.values(action);
    }

    return QList<KisShortcutConfiguration *>();
}

void KisInputProfile::addShortcut(KisShortcutConfiguration *shortcut)
{
    Q_ASSERT(shortcut);
    Q_ASSERT(shortcut->action());
    d->shortcuts.insert(shortcut->action(), shortcut);
}

void KisInputProfile::removeShortcut(KisShortcutConfiguration *shortcut)
{
    Q_ASSERT(shortcut);
    Q_ASSERT(shortcut->action());
    d->shortcuts.remove(shortcut->action(), shortcut);
}
