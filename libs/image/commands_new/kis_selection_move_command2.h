/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SELECTION_MOVE_COMMAND2_H
#define KIS_SELECTION_MOVE_COMMAND2_H

#include "kis_move_command_common.h"

/**
 * A specialized class for moving a selection without its flattening and recalculation
 * of the outline cache. See details in a comment to KisMoveCommandCommon.
 */
class KRITAIMAGE_EXPORT KisSelectionMoveCommand2 : public KisMoveCommandCommon<KisSelectionSP>
{
public:
    KisSelectionMoveCommand2(KisSelectionSP object, const QPoint& oldPos, const QPoint& newPos, KUndo2Command *parent = 0);

    void undo() override;
    void redo() override;
};

#endif
