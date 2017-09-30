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


struct Action::Private {
    Private() {}
    QAction *action {0};
};

Action::Action(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->action = new KisAction(this);
    connect(d->action, SIGNAL(triggered(bool)), SIGNAL(triggered(bool)));
}

Action::Action(const QString &name, QAction *action, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->action = action;
    d->action->setObjectName(name);
    connect(d->action, SIGNAL(triggered(bool)), SIGNAL(triggered(bool)));
}

Action::~Action()
{
    delete d;
}


bool Action::operator==(const Action &other) const
{
    return (d->action == other.d->action);
}

bool Action::operator!=(const Action &other) const
{
    return !(operator==(other));
}


QString Action::text() const
{
    if (!d->action) return "";
    return d->action->text();
}

void Action::setText(QString text)
{
    if (!d->action) return;
    d->action->setText(text);
}

QString Action::name() const
{
    if (!d->action) return "";
    return d->action->objectName();
}

void Action::setName(QString name)
{
    if (!d->action) return;
    d->action->setObjectName(name);
}

bool Action::isCheckable() const
{
    if (!d->action) return false;
    return d->action->isCheckable();
}

void Action::setCheckable(bool value)
{
    if (!d->action) return;
    d->action->setCheckable(value);
}

bool Action::isChecked() const
{
    if (!d->action) return false;
    return d->action->isChecked();
}

void Action::setChecked(bool value)
{
    if (!d->action) return;
    d->action->setChecked(value);
}

QString Action::shortcut() const
{
    if (!d->action) return QString();
    return d->action->shortcut().toString();
}

void Action::setShortcut(QString value)
{
    if (!d->action) return;
    d->action->setShortcut(QKeySequence::fromString(value));
}

bool Action::isVisible() const
{
    if (!d->action) return false;
    return d->action->isVisible();
}

void Action::setVisible(bool value)
{
    if (!d->action) return;
    d->action->setVisible(value);
}

bool Action::isEnabled() const
{
    if (!d->action) return false;
    return d->action->isEnabled();
}

void Action::setEnabled(bool value)
{
    if (!d->action) return;
    d->action->setEnabled(value);
}

void Action::setToolTip(QString tooltip)
{
    if (!d->action) return;
    d->action->setToolTip(tooltip);
}

QString Action::tooltip() const
{
    return d->action->toolTip();
}

void Action::trigger()
{
    d->action->trigger();
}


void Action::setMenu(const QString menu)
{
    d->action->setProperty("menu", menu);
}

QString Action::menu() const
{
    return d->action->property("menu").toString();
}
