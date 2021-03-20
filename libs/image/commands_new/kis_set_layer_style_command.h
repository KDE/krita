/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SET_LAYER_STYLE_COMMAND_H
#define KIS_SET_LAYER_STYLE_COMMAND_H

#include <klocalizedstring.h>
#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_psd_layer_style.h"

class KRITAIMAGE_EXPORT KisSetLayerStyleCommand : public KUndo2Command
{
public:
    KisSetLayerStyleCommand(KisLayerSP layer, KisPSDLayerStyleSP oldStyle, KisPSDLayerStyleSP newStyle, KUndo2Command *parent = 0);

    void undo() override;
    void redo() override;

    static void updateLayerStyle(KisLayerSP layer, KisPSDLayerStyleSP style);

private:
    KisLayerSP m_layer;
    KisPSDLayerStyleSP m_oldStyle;
    KisPSDLayerStyleSP m_newStyle;
};

#endif
