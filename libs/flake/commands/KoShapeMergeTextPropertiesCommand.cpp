/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoShapeMergeTextPropertiesCommand.h"

#include <KoSvgTextShape.h>
#include <KoSvgTextProperties.h>
#include <kis_command_ids.h>

struct KoShapeMergeTextPropertiesCommand::Private {
    Private(const QList<KoShape*> &list) : shapes(list) { }
    QList<KoShape *> shapes;
    QMap<KoShape*, KoSvgTextShapeMementoSP> mementos;
    KoSvgTextProperties newProperties;
};

KoShapeMergeTextPropertiesCommand::KoShapeMergeTextPropertiesCommand(const QList<KoShape *> &shapes, const KoSvgTextProperties &props, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new Private(shapes))
{
    d->newProperties = props;
}

void KoShapeMergeTextPropertiesCommand::redo()
{
    for(auto it = d->shapes.begin(); it!= d->shapes.end(); it++) {
        KoSvgTextShape *shape = dynamic_cast<KoSvgTextShape*>(*it);
        if (shape) {
            d->mementos.insert(*it, shape->getMemento());
            const QRectF oldDirtyRect = shape->boundingRect();
            shape->mergePropertiesIntoRange(-1, -1, d->newProperties);
            shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
        }
    }
}

void KoShapeMergeTextPropertiesCommand::undo()
{
    for(auto it = d->shapes.begin(); it!= d->shapes.end(); it++) {
        KoSvgTextShape *shape = dynamic_cast<KoSvgTextShape*>(*it);
        if (shape && d->mementos.contains(*it)) {
            const QRectF oldDirtyRect = shape->boundingRect();
            shape->setMemento(d->mementos.value(*it));
            shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
        }
    }
}

int KoShapeMergeTextPropertiesCommand::id() const
{
    return KisCommandUtils::KoShapeMergeTextPropertiesCommandId;
}

bool KoShapeMergeTextPropertiesCommand::mergeWith(const KUndo2Command *other)
{
    const KoShapeMergeTextPropertiesCommand *command = dynamic_cast<const KoShapeMergeTextPropertiesCommand*>(other);

    if (!command || command->d->shapes != d->shapes) {
        return false;
    }

    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->d->newProperties.properties()) {
        d->newProperties.setProperty(p, command->d->newProperties.property(p));
    }

    return true;
}
