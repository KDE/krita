/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_PAINTOP_PRESETS_SAVE_H
#define KIS_PAINTOP_PRESETS_SAVE_H

#include <QWidget>
#include <QDialog>

#include "ui_wdgsavebrushpreset.h"
#include "kis_canvas_resource_provider.h"
#include "kis_favorite_resource_manager.h"

class KisPaintOpPresetSaveDialog : public QDialog , public Ui::WdgSaveBrushPreset
{
    Q_OBJECT

public:
    KisPaintOpPresetSaveDialog(QWidget* parent) : QDialog(parent) {
        setupUi(this);
    }       
};


class KisPresetSaveWidget : public KisPaintOpPresetSaveDialog
{
    Q_OBJECT

public:
    KisPresetSaveWidget(QWidget* parent);
    virtual ~KisPresetSaveWidget();

    void showDialog();

    /// determines if we should show the save as dialog (true) or save in the background (false)
    void useNewBrushDialog(bool show);

    void scratchPadSetup(KisCanvasResourceProvider* resourceProvider);
    void saveScratchPadThumbnailArea(const QImage image);
    KisCanvasResourceProvider* m_resourceProvider;

    void setFavoriteResourceManager(KisFavoriteResourceManager * favManager);

Q_SIGNALS:
    void resourceSelected(KoResourceSP resource);


public Q_SLOTS:

    void loadImageFromFile();
    void savePreset();
    void loadScratchpadThumbnail();
    void loadExistingThumbnail();
    void loadImageFromLibrary();


private:
    bool m_useNewBrushDialog;
    KisFavoriteResourceManager * m_favoriteResourceManager;
    QImage scratchPadThumbnailArea;
};


#endif
