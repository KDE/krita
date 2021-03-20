/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeSizeCommand.h"

#include <KoShape.h>
#include <klocalizedstring.h>

class Q_DECL_HIDDEN KoShapeSizeCommand::Private
{
public:
    QList<KoShape*> shapes;
    QList<QSizeF> previousSizes, newSizes;
};

KoShapeSizeCommand::KoShapeSizeCommand(const QList<KoShape*> &shapes, const QList<QSizeF> &previousSizes, const QList<QSizeF> &newSizes, KUndo2Command *parent)
        : KUndo2Command(parent),
        d(new Private())
{
    d->previousSizes = previousSizes;
    d->newSizes = newSizes;
    d->shapes = shapes;
    Q_ASSERT(d->shapes.count() == d->previousSizes.count());
    Q_ASSERT(d->shapes.count() == d->newSizes.count());

    setText(kundo2_i18n("Resize shapes"));
}

KoShapeSizeCommand::~KoShapeSizeCommand()
{
    delete d;
}

void KoShapeSizeCommand::redo()
{
    KUndo2Command::redo();
    int i = 0;
    Q_FOREACH (KoShape *shape, d->shapes) {
        shape->update();
        shape->setSize(d->newSizes[i++]);
        shape->update();
    }
}

void KoShapeSizeCommand::undo()
{
    KUndo2Command::undo();
    int i = 0;
    Q_FOREACH (KoShape *shape, d->shapes) {
        shape->update();
        shape->setSize(d->previousSizes[i++]);
        shape->update();
    }
}
