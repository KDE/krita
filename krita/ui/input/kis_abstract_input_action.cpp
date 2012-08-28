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
#include <KLocalizedString>

class KisAbstractInputAction::Private
{
public:
    KisInputManager* inputManager;

    QString name;
    QString description;
    QHash<QString, int> indexes;

    QPointF mousePosition;
};

KisAbstractInputAction::KisAbstractInputAction(KisInputManager* manager) : d(new Private)
{
    d->inputManager = manager;
    d->indexes.insert(i18n("Activate"), 0);
}

KisAbstractInputAction::~KisAbstractInputAction()
{
    delete d;
}

bool KisAbstractInputAction::handleTablet() const
{
    return false;
}

KisInputManager* KisAbstractInputAction::inputManager() const
{
    return d->inputManager;
}

QString KisAbstractInputAction::name() const
{
    return d->name;
}

QString KisAbstractInputAction::description() const
{
    return d->description;
}

QHash< QString, int > KisAbstractInputAction::shortcutIndexes() const
{
    return d->indexes;
}

void KisAbstractInputAction::setName(const QString& name)
{
    d->name = name;
}

void KisAbstractInputAction::setDescription(const QString& description)
{
    d->description = description;
}

void KisAbstractInputAction::setShortcutIndexes(const QHash< QString, int >& indexes)
{
    d->indexes = indexes;
}

bool KisAbstractInputAction::isBlockingAutoRepeat() const
{
    return false;
}

QPointF KisAbstractInputAction::mousePosition() const
{
    return d->mousePosition;
}

void KisAbstractInputAction::setMousePosition(const QPointF &position)
{
    d->mousePosition = position;
}
