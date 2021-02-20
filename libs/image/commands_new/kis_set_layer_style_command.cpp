/*
 *  SPDX-FileCopyrightText: 2014 Stuart Dickson <stuartmd@kogmbh.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_set_layer_style_command.h"

#include "kis_selection_mask.h"
#include "kis_layer.h"
#include "kis_abstract_projection_plane.h"
#include "kis_psd_layer_style.h"


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
