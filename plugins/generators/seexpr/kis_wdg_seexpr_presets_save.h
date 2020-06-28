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

#ifndef KIS_WDG_SEEXPR_PRESETS_SAVE_H
#define KIS_WDG_SEEXPR_PRESETS_SAVE_H

#include <QDialog>
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
    void resourceSelected(KoResource* resource);

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
