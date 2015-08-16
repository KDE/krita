/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_SET_LAYER_STYLE_COMMAND_H
#define KIS_SET_LAYER_STYLE_COMMAND_H

#include <klocale.h>
#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"


class KRITAIMAGE_EXPORT KisSetLayerStyleCommand : public KUndo2Command
{
public:
    KisSetLayerStyleCommand(KisLayerSP layer, KisPSDLayerStyleSP oldStyle, KisPSDLayerStyleSP newStyle, KUndo2Command *parent = 0);

    void undo();
    void redo();

    static void updateLayerStyle(KisLayerSP layer, KisPSDLayerStyleSP style);

private:
    KisLayerSP m_layer;
    KisPSDLayerStyleSP m_oldStyle;
    KisPSDLayerStyleSP m_newStyle;
};

#endif
