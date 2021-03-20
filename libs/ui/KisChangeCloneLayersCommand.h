/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CHANGE_CLONE_LAYERS_COMMAND_H_
#define KIS_CHANGE_CLONE_LAYERS_COMMAND_H_

#include <kundo2command.h>
#include "kis_types.h"

class KisChangeCloneLayersCommand : public KUndo2Command
{

public:
    KisChangeCloneLayersCommand(QList<KisCloneLayerSP> cloneLayers, KisLayerSP newSource, KUndo2Command *parent = 0);

    void undo() override;
    void redo() override;
    bool mergeWith(const KUndo2Command *) override;

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KIS_CHANGE_CLONE_LAYERS_COMMAND_H_
