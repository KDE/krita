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
    QSet<KoSvgTextProperties::PropertyId> removeProperties;

};

KoShapeMergeTextPropertiesCommand::KoShapeMergeTextPropertiesCommand(const QList<KoShape *> &shapes, const KoSvgTextProperties &props, const QSet<KoSvgTextProperties::PropertyId> removeProperties, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new Private(shapes))
{
    d->newProperties = props;
    d->removeProperties = removeProperties;
}

void KoShapeMergeTextPropertiesCommand::redo()
{
    for(auto it = d->shapes.begin(); it!= d->shapes.end(); it++) {
        KoSvgTextShape *shape = dynamic_cast<KoSvgTextShape*>(*it);
        if (shape) {
            d->mementos.insert(*it, shape->getMemento());
            const QRectF oldDirtyRect = shape->boundingRect();
            shape->mergePropertiesIntoRange(-1, -1, d->newProperties, d->removeProperties);
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

    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->d->removeProperties) {
        d->newProperties.removeProperty(p);
        d->removeProperties.insert(p);
    }
    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->d->newProperties.properties()) {
        if (!command->d->removeProperties.contains(p)) {
            d->newProperties.setProperty(p, command->d->newProperties.property(p));
            d->removeProperties.remove(p);
        }
    }

    return true;
}
