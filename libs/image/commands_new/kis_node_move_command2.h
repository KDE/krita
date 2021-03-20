/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_NODE_MOVE_COMMAND2_H
#define KIS_NODE_MOVE_COMMAND2_H

#include "kis_move_command_common.h"

class KRITAIMAGE_EXPORT KisNodeMoveCommand2 : public KisMoveCommandCommon<KisNodeSP>
{
public:
    KisNodeMoveCommand2(KisNodeSP object, const QPoint& oldPos, const QPoint& newPos, KUndo2Command *parent = 0);

    void undo() override;
    void redo() override;

    static void tryNotifySelection(KisNodeSP node);
};

#endif
