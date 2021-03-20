/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOMULTIPATHPOINTJOINCOMMAND_H
#define KOMULTIPATHPOINTJOINCOMMAND_H

#include <KoMultiPathPointMergeCommand.h>

class KRITAFLAKE_EXPORT KoMultiPathPointJoinCommand : public KoMultiPathPointMergeCommand
{
public:
    KoMultiPathPointJoinCommand(const KoPathPointData &pointData1,
                                const KoPathPointData &pointData2,
                                KoShapeControllerBase *controller,
                                KoSelection *selection,
                                KUndo2Command *parent = 0);

protected:
    KUndo2Command *createMergeCommand(const KoPathPointData &pointData1,
                                              const KoPathPointData &pointData2) override;
};

#endif // KOMULTIPATHPOINTJOINCOMMAND_H
