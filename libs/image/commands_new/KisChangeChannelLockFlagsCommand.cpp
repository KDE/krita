/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisChangeChannelLockFlagsCommand.h"
#include "kis_paint_layer.h"

#include <klocalizedstring.h>

KisChangeChannelLockFlagsCommand::KisChangeChannelLockFlagsCommand(const QBitArray &newFlags, const QBitArray &oldFlags,
                                                                   KisPaintLayerSP layer, KUndo2Command *parentCommand)
    : KUndo2Command(kundo2_noi18n("change-channel-lock-flags-command"), parentCommand),
      m_layer(layer),
      m_oldFlags(oldFlags),
      m_newFlags(newFlags)
{
}

KisChangeChannelLockFlagsCommand::KisChangeChannelLockFlagsCommand(const QBitArray &newFlags, KisPaintLayerSP layer, KUndo2Command *parentCommand)
    : KisChangeChannelLockFlagsCommand(newFlags, layer->channelLockFlags(), layer, parentCommand)
{
}

void KisChangeChannelLockFlagsCommand::redo()
{
    m_layer->setChannelLockFlags(m_newFlags);
}

void KisChangeChannelLockFlagsCommand::undo()
{
    m_layer->setChannelLockFlags(m_oldFlags);
}
