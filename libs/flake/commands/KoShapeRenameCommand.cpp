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
#include <klocalizedstring.h>
#include "KoShape.h"

class Q_DECL_HIDDEN KoShapeRenameCommand::Private
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

KoShapeRenameCommand::KoShapeRenameCommand(KoShape *shape, const QString &newName, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Rename Shape"), parent)
, d(new Private(shape, newName))
{
}

KoShapeRenameCommand::~KoShapeRenameCommand()
{
   delete d;
}

void KoShapeRenameCommand::redo()
{
    KUndo2Command::redo();
    d->shape->setName(d->newName);
}

void KoShapeRenameCommand::undo()
{
    KUndo2Command::undo();
    d->shape->setName(d->oldName);
}
