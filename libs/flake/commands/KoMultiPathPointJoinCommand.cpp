/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoMultiPathPointJoinCommand.h"

#include <KoSubpathJoinCommand.h>

KoMultiPathPointJoinCommand::KoMultiPathPointJoinCommand(const KoPathPointData &pointData1,
                                                         const KoPathPointData &pointData2,
                                                         KoShapeControllerBase *controller,
                                                         KoSelection *selection,
                                                         KUndo2Command *parent)
    : KoMultiPathPointMergeCommand(pointData1, pointData2, controller, selection, parent)
{
    setText(kundo2_i18n("Join subpaths"));
}

KUndo2Command *KoMultiPathPointJoinCommand::createMergeCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2)
{
    return new KoSubpathJoinCommand(pointData1, pointData2);
}

