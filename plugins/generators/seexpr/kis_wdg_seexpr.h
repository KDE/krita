/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    void slotResourceSaved(KoResourceSP resource);
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
