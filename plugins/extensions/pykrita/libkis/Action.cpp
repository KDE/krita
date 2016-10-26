/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Action.h"

#include <QAction>

struct Action::Private {
    Private() {}
    QAction *action {0};
    QString name;
    QString menu;
};

Action::Action(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->action = new QAction(this);
}

Action::Action(const QString &name, QAction *action, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->name = name;
    d->action = action;
}

Action::~Action()
{
    delete d;
}

QString Action::name() const
{
    return d->name;
}

void Action::setName(QString value)
{
    d->name = value;
}


QString Action::menu() const
{
    return d->menu;
}

void Action::setMenu(QString value)
{
    d->menu = value;
}

bool Action::isCheckable() const
{
    return d->action->isCheckable();
}

void Action::setCheckable(bool value)
{
    d->action->setCheckable(value);
}

bool Action::isChecked() const
{
    return d->action->isChecked();
}

void Action::setChecked(bool value)
{
    d->action->setChecked(value);
}


QString Action::shortcut() const
{
    return d->action->shortcut().toString();
}

void Action::setShortcut(QString value)
{
    d->action->setShortcut(QKeySequence::fromString(value));
}


bool Action::isVisible() const
{
    return d->action->isVisible();
}

void Action::setVisible(bool value)
{
    d->action->setVisible(value);
}


bool Action::isEnabled() const
{
    return d->action->isEnabled();
}

void Action::setEnabled(bool value)
{
    d->action->setEnabled(value);
}

void Action::trigger()
{
    d->action->trigger();
}



