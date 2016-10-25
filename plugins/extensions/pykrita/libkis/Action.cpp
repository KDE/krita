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
};

Action::Action(QObject *parent) 
    : QObject(parent)
    , d(new Private)
{
}

Action::~Action() 
{
    delete d;
}

QString Action::name() const
{
    return QString();
}

void Action::setName(QString value)
{
}


QString Action::menu() const
{
    return QString();
}

void Action::setMenu(QString value)
{
}


bool Action::checkable() const
{
    return false;
}

void Action::setCheckable(bool value)
{
}


bool Action::checked() const
{
    return false;
}

void Action::setChecked(bool value)
{
}


QString Action::shortcut() const
{
    return QString();
}

void Action::setShortcut(QString value)
{
}


bool Action::visible() const
{
    return false;
}

void Action::setVisible(bool value)
{
}


bool Action::enabled() const
{
    return false;
}

void Action::setEnabled(bool value)
{
}




void Action::Trigger()
{
}

void Action::Toggle(bool toggle)
{
}



