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

#include "KoShapeRenameCommand.h"

#include <QString>
#include <klocale.h>
#include "KoShape.h"

class KoShapeRenameCommand::Private
{
public:
    Private(KoShape *shape, const QString &newName)
    : shape(shape)
    , newName(newName)
    , oldName(shape->name())
    {}

    KoShape *shape;
    QString newName;
    QString oldName;
};

KoShapeRenameCommand::KoShapeRenameCommand(KoShape *shape, const QString &newName, QUndoCommand *parent)
: QUndoCommand(parent)
, d(new Private(shape, newName))
{
    // TODO 2.1 add "Rename Shape"
    //setText();
}

KoShapeRenameCommand::~KoShapeRenameCommand()
{
   delete d;
}

void KoShapeRenameCommand::redo()
{
    QUndoCommand::redo();
    d->shape->setName(d->newName);
}

void KoShapeRenameCommand::undo()
{
    QUndoCommand::undo();
    d->shape->setName(d->oldName);
}
