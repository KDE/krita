/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
