/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoEventActionAddCommand.h"
#include <klocale.h>

#include "KoShape.h"
#include "KoEventAction.h"

class KoEventActionAddCommandPrivate
{
public:
    KoEventActionAddCommandPrivate(KoShape *s, KoEventAction *a)
        : shape(s), eventAction(a), deleteEventAction(true)
    {
    }

    ~KoEventActionAddCommandPrivate() {
        if (deleteEventAction)
            delete eventAction;
    }
    KoShape *shape;
    KoEventAction *eventAction;
    bool deleteEventAction;
};


KoEventActionAddCommand::KoEventActionAddCommand(KoShape *shape, KoEventAction *eventAction, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoEventActionAddCommandPrivate(shape, eventAction))
{
}

KoEventActionAddCommand::~KoEventActionAddCommand()
{
    delete d;
}

void KoEventActionAddCommand::redo()
{
    d->shape->addEventAction(d->eventAction);
    d->deleteEventAction = false;
}

void KoEventActionAddCommand::undo()
{
    d->shape->removeEventAction(d->eventAction);
    d->deleteEventAction = true;
}
