/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisChangeCloneLayersCommand.h"

#include <kis_clone_layer.h>

struct KisChangeCloneLayersCommand::Private
{
    QList<KisCloneLayerSP> cloneLayers;
    QList<KisLayerSP> originalSource;
    KisLayerSP newSource;
};

KisChangeCloneLayersCommand::KisChangeCloneLayersCommand(QList<KisCloneLayerSP> cloneLayers, KisLayerSP newSource, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Change Clone Layers"), parent)
    , d(new Private())
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!cloneLayers.isEmpty());
    d->cloneLayers = cloneLayers;
    Q_FOREACH (KisCloneLayerSP layer, d->cloneLayers) {
        d->originalSource << layer->copyFrom();
    }
    d->newSource = newSource;
}

void KisChangeCloneLayersCommand::redo()
{
    Q_FOREACH (KisCloneLayerSP layer, d->cloneLayers) {
        layer->setCopyFrom(d->newSource);
        layer->setDirty();
    }
}

void KisChangeCloneLayersCommand::undo()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->cloneLayers.size() == d->originalSource.size());
    for (int i = 0; i < d->cloneLayers.size(); ++i) {
        KisCloneLayerSP layer = d->cloneLayers.at(i);
        layer->setCopyFrom(d->originalSource.at(i));
        layer->setDirty();
    }
}

bool KisChangeCloneLayersCommand::mergeWith(const KUndo2Command *command)
{
    const KisChangeCloneLayersCommand *other = dynamic_cast<const KisChangeCloneLayersCommand *>(command);

    if (other && d->cloneLayers == other->d->cloneLayers) {
        d->newSource = other->d->newSource;
        return true;
    }

    return false;
}
