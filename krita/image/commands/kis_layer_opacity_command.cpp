/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include "commands/kis_layer_commands.h"
#include <klocale.h>

#include <KoCompositeOp.h>

#include "kis_layer.h"

KisLayerOpacityCommand::KisLayerOpacityCommand(KisLayerSP layer, quint8 oldOpacity, quint8 newOpacity) :
        KisLayerCommand(i18n("Layer Opacity"), layer)
{
    m_oldOpacity = oldOpacity;
    m_newOpacity = newOpacity;
}

void KisLayerOpacityCommand::redo()
{
    m_layer->setOpacity(m_newOpacity);
    m_layer->setDirty();
}

void KisLayerOpacityCommand::undo()
{
    m_layer->setOpacity(m_oldOpacity);
    m_layer->setDirty();
}
