/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
