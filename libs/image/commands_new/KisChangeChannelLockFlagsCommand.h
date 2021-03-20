/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_CHANGE_CHANNEL_LOCK_FLAGS_COMMAND_H_
#define KIS_CHANGE_CHANNEL_LOCK_FLAGS_COMMAND_H_

#include <kritaimage_export.h>

#include <QBitArray>

#include "kis_types.h"
#include <kundo2command.h>

class KisChangeChannelLockFlagsCommand : public KUndo2Command
{

public:
    KisChangeChannelLockFlagsCommand(const QBitArray &newFlags,
                                     KisPaintLayerSP layer,
                                     KUndo2Command *parentCommand = 0);


    KisChangeChannelLockFlagsCommand(const QBitArray &newFlags,
                                     const QBitArray &oldFlags,
                                     KisPaintLayerSP layer,
                                     KUndo2Command *parentCommand = 0);

    void redo() override;
    void undo() override;

protected:
    KisPaintLayerSP m_layer;
    QBitArray m_oldFlags;
    QBitArray m_newFlags;
};

#endif
