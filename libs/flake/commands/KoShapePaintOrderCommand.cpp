/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapePaintOrderCommand.h"

#include <klocalizedstring.h>
#include "kis_command_ids.h"

class Q_DECL_HIDDEN KoShapePaintOrderCommand::Private
{
public:
    Private() {
    }
    ~Private() {
    }

    QList<KoShape*> shapes;
    QList<KoShape::PaintOrder> oldFirst;
    QList<KoShape::PaintOrder> oldSecond;
    QList<bool> paintOrderInherited;
    KoShape::PaintOrder first;
    KoShape::PaintOrder second;
};

KoShapePaintOrderCommand::KoShapePaintOrderCommand(const QList<KoShape *> &shapes,
                                                   KoShape::PaintOrder first,
                                                   KoShape::PaintOrder second,
                                                   KUndo2Command *parent)
    : KUndo2Command(parent)
    , d(new Private())
{
    d->shapes = shapes;
    Q_FOREACH (KoShape *shape, d->shapes) {
        d->oldFirst.append(shape->paintOrder().at(0));
        d->oldSecond.append(shape->paintOrder().at(1));
        d->paintOrderInherited.append(shape->inheritPaintOrder());
    }
    d->first = first;
    d->second = second;

    setText(kundo2_i18n("Set paint order"));
}

KoShapePaintOrderCommand::~KoShapePaintOrderCommand()
{
    delete d;
}


void KoShapePaintOrderCommand::redo()
{
    KUndo2Command::redo();
    Q_FOREACH (KoShape *shape, d->shapes) {
        shape->setPaintOrder(d->first, d->second);
        shape->update();
    }
}

void KoShapePaintOrderCommand::undo()
{
    KUndo2Command::undo();
    QList<KoShape::PaintOrder>::iterator firstIt = d->oldFirst.begin();
    QList<KoShape::PaintOrder>::iterator secondIt = d->oldSecond.begin();
    QList<bool>::iterator inheritIt = d->paintOrderInherited.begin();
    Q_FOREACH (KoShape *shape, d->shapes) {
        shape->setPaintOrder(*firstIt, *secondIt);
        shape->setInheritPaintOrder(*inheritIt);
        shape->update();
        ++firstIt;
        ++secondIt;
        ++inheritIt;
    }
}

int KoShapePaintOrderCommand::id() const
{
    return KisCommandUtils::ChangePaintOrderCommand;
}

bool KoShapePaintOrderCommand::mergeWith(const KUndo2Command *command)
{
    const KoShapePaintOrderCommand *other = dynamic_cast<const KoShapePaintOrderCommand*>(command);

    if (!other || other->d->shapes != d->shapes) {
        return false;
    }

    d->first = other->d->first;
    d->second = other->d->second;
    return true;
}
