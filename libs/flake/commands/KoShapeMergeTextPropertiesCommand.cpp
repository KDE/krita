/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoShapeMergeTextPropertiesCommand.h"

#include <KoSvgTextShape.h>
#include <KoSvgTextProperties.h>
#include <kis_command_ids.h>
#include <krita_container_utils.h>
#include <KoShapeBulkActionLock.h>

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
    setText(kundo2_i18n("Change Text Shape Properties"));
    d->newProperties = props;
    d->removeProperties = removeProperties;
}

void KoShapeMergeTextPropertiesCommand::redo()
{
    KoShapeBulkActionLock lock(d->shapes);

    for(auto it = d->shapes.begin(); it!= d->shapes.end(); it++) {
        KoSvgTextShape *shape = dynamic_cast<KoSvgTextShape*>(*it);
        if (shape) {
            d->mementos.insert(*it, shape->getMemento());
            shape->mergePropertiesIntoRange(-1, -1, d->newProperties, d->removeProperties);
        }
    }

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

void KoShapeMergeTextPropertiesCommand::undo()
{
    KoShapeBulkActionLock lock(d->shapes);

    for(auto it = d->shapes.begin(); it!= d->shapes.end(); it++) {
        KoSvgTextShape *shape = dynamic_cast<KoSvgTextShape*>(*it);
        if (shape && d->mementos.contains(*it)) {
            shape->setMemento(d->mementos.value(*it));
        }
    }
    d->mementos.clear();

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

int KoShapeMergeTextPropertiesCommand::id() const
{
    return KisCommandUtils::KoShapeMergeTextPropertiesCommandId;
}

bool KoShapeMergeTextPropertiesCommand::mergeWith(const KUndo2Command *other)
{
    const KoShapeMergeTextPropertiesCommand *command = dynamic_cast<const KoShapeMergeTextPropertiesCommand*>(other);

    if (!command || !KritaUtils::compareListsUnordered(command->d->shapes, d->shapes)) {
        return false;
    }

    /**
     * The merging algorithm should follow the ordering of
     * KoSvgTextShape::mergePropertiesIntoRange, that is, firstly,
     * the properties in @p removeProperties list are removed,
     * then properties in @p properties are applied. If the property is
     * present in both lists, then the value from @p properties is used.
     */

    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->d->removeProperties) {
        d->newProperties.removeProperty(p);
        d->removeProperties.insert(p);
    }

    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->d->newProperties.properties()) {
        d->newProperties.setProperty(p, command->d->newProperties.property(p));
        d->removeProperties.remove(p);
    }

    return true;
}
