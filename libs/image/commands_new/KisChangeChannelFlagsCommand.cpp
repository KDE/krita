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

#include "KisChangeChannelFlagsCommand.h"
#include "kis_layer.h"

#include <klocalizedstring.h>

KisChangeChannelFlagsCommand::KisChangeChannelFlagsCommand(const QBitArray &newFlags, const QBitArray &oldFlags,
                                                           KisLayerSP layer, KUndo2Command *parentCommand)
    : KUndo2Command(kundo2_noi18n("change-channel-flags-command"), parentCommand),
      m_layer(layer),
      m_oldFlags(oldFlags),
      m_newFlags(newFlags)
{
}

KisChangeChannelFlagsCommand::KisChangeChannelFlagsCommand(const QBitArray &newFlags, KisLayerSP layer, KUndo2Command *parentCommand)
    : KisChangeChannelFlagsCommand(newFlags, layer->channelFlags(), layer, parentCommand)
{
}

void KisChangeChannelFlagsCommand::redo()
{
    m_layer->setChannelFlags(m_newFlags);
}

void KisChangeChannelFlagsCommand::undo()
{
    m_layer->setChannelFlags(m_oldFlags);
}
