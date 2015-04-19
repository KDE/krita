/*
 *  Copyright (c) 2014 Stuart Dickson <stuartmd@kogmbh.com>
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
#include "kis_set_layer_style_command.h"

#include "kis_selection_mask.h"
#include "kis_layer.h"
#include "kis_abstract_projection_plane.h"


KisSetLayerStyleCommand::KisSetLayerStyleCommand(KisLayerSP layer, KisPSDLayerStyleSP oldStyle, KisPSDLayerStyleSP newStyle, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Change Layer Style"), parent),
      m_layer(layer),
      m_oldStyle(oldStyle),
      m_newStyle(newStyle)
{
}

void KisSetLayerStyleCommand::redo()
{
    updateLayerStyle(m_layer, m_newStyle);
}

void KisSetLayerStyleCommand::undo()
{
    updateLayerStyle(m_layer, m_oldStyle);
}

void KisSetLayerStyleCommand::updateLayerStyle(KisLayerSP layer, KisPSDLayerStyleSP style)
{
    QRect oldDirtyRect = layer->projectionPlane()->changeRect(layer->extent(), KisLayer::N_FILTHY);
    layer->setLayerStyle(style);
    QRect newDirtyRect = layer->projectionPlane()->changeRect(layer->extent(), KisLayer::N_FILTHY);

    layer->setDirty(newDirtyRect | oldDirtyRect);
}
