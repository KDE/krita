/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#ifndef KIS_WDG_SEEXPR_H
#define KIS_WDG_SEEXPR_H

#include <kis_config_widget.h>
#include <kis_signal_compressor.h>
#include <resources/KisSeExprScript.h>

#include "kis_wdg_seexpr_presets_save.h"

class Ui_WdgSeExpr;

class KisWdgSeExpr : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgSeExpr(QWidget *parent = 0);
    ~KisWdgSeExpr() override;

public:
    inline const Ui_WdgSeExpr *widget() const;
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:
    void isValid();
    void slotResourceSelected(KoResourceSP resource);
    void slotRenamePresetActivated();
    void slotRenamePresetDeactivated();
    void slotSaveRenameCurrentPreset();
    void slotUpdatePresetSettings();
    void slotSaveBrushPreset();
    void slotSaveNewBrushPreset();
    void slotReloadPresetClicked();

private:
    Ui_WdgSeExpr *m_widget;
    KisSignalCompressor updateCompressor;
    KisSeExprScriptSP m_currentPreset;
    KisWdgSeExprPresetsSave *m_saveDialog;

    bool m_isCreatingPresetFromScratch;

    void togglePresetRenameUIActive(bool isRenaming);
};

#endif
