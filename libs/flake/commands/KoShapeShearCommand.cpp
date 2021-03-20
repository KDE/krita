/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeShearCommand.h"
#include "KoShape.h"

#include <klocalizedstring.h>

class KoShapeShearCommandPrivate
{
public:
    QList<KoShape*> shapes;
    QList<qreal> previousShearXs;
    QList<qreal> previousShearYs;
    QList<qreal> newShearXs;
    QList<qreal> newShearYs;
};

KoShapeShearCommand::KoShapeShearCommand(const QList<KoShape*> &shapes, const QList<qreal> &previousShearXs, const QList<qreal> &previousShearYs, const QList<qreal> &newShearXs, const QList<qreal> &newShearYs, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new KoShapeShearCommandPrivate())
{
    d->shapes = shapes;
    d->previousShearXs = previousShearXs;
    d->previousShearYs = previousShearYs;
    d->newShearXs = newShearXs;
    d->newShearYs = newShearYs;

    Q_ASSERT(d->shapes.count() == d->previousShearXs.count());
    Q_ASSERT(d->shapes.count() == d->previousShearYs.count());
    Q_ASSERT(d->shapes.count() == d->newShearXs.count());
    Q_ASSERT(d->shapes.count() == d->newShearYs.count());

    setText(kundo2_i18n("Shear shapes"));
}

KoShapeShearCommand::~KoShapeShearCommand()
{
    delete d;
}

void KoShapeShearCommand::redo()
{
    KUndo2Command::redo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->shear(d->newShearXs.at(i), d->newShearYs.at(i));
        d->shapes.at(i)->update();
    }
}

void KoShapeShearCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->shear(d->previousShearXs.at(i), d->previousShearYs.at(i));
        d->shapes.at(i)->update();
    }
}
