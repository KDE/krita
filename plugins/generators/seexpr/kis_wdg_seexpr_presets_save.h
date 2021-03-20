/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_SEEXPR_PRESETS_SAVE_H
#define KIS_WDG_SEEXPR_PRESETS_SAVE_H

#include <QDialog>
#include <filter/kis_filter_configuration.h>
#include <resources/KisSeExprScript.h>

#include "ui_wdgseexprsavepreset.h"

/**
 * Dialog for saving SeExpr presets.
 *
 * Based on Scott Petrovic's KisPaintOpPresetSaveDialog.
 */
class KisWdgSeExprSavePreset : public QDialog, public Ui::WdgSeExprSavePreset
{
    Q_OBJECT

public:
    KisWdgSeExprSavePreset(QWidget *parent)
        : QDialog(parent)
    {
        setupUi(this);
    }
};

class KisWdgSeExprPresetsSave : public KisWdgSeExprSavePreset
{
    Q_OBJECT

public:
    KisWdgSeExprPresetsSave(QWidget *parent);
    virtual ~KisWdgSeExprPresetsSave();

    void showDialog();

    /// determines if we should show the save as dialog (true) or save in the background (false)
    void useNewPresetDialog(bool show);

Q_SIGNALS:
    // Triggers after resource is saved
    void resourceSelected(KoResourceSP resource);

public Q_SLOTS:
    void setCurrentPreset(KisSeExprScriptSP resource);
    void setCurrentRenderConfiguration(KisFilterConfigurationSP config);
    void loadImageFromFile();
    void savePreset();
    void loadExistingThumbnail();
    void renderScriptToThumbnail();

private:
    bool m_useNewPresetDialog;
    KisSeExprScriptSP m_currentPreset;
    KisFilterConfigurationSP m_currentConfiguration;
};

#endif
