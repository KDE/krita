/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PAINTOP_PRESETS_POPUP_H
#define KIS_PAINTOP_PRESETS_POPUP_H

#include <QWidget>
#include <QList>
#include <KoID.h>
#include <kis_types.h>
#include <brushengine/kis_paintop_factory.h>
#include "../kis_paint_ops_model.h"
#include <kis_action.h>
#include <widgets/kis_paintop_presets_save.h>
#include "widgets/kis_paintop_presets_popup.h"
#include "kis_favorite_resource_manager.h"

class QString;
class KisCanvasResourceProvider;
class KoResource;

/**
 * Popup widget for presets with built-in functionality
 * for adding and removing presets.
 */
class KisPaintOpPresetsPopup : public QWidget
{
    Q_OBJECT

public:

    KisPaintOpPresetsPopup(KisCanvasResourceProvider * resourceProvider,
                           KisFavoriteResourceManager* favoriteResourceManager,
                           KisPresetSaveWidget* savePresetWidget,
                           QWidget * parent = 0);

    ~KisPaintOpPresetsPopup() override;

    void setPaintOpSettingsWidget(QWidget * widget);


    ///Image for preset preview
    ///@return image cut out from the scratchpad
    QImage cutOutOverlay();

    void setPaintOpList(const QList<KisPaintOpFactory*>& list);

    void setCurrentPaintOpId(const QString & paintOpId);

    /// returns the internal ID for the paint op (brush engine)
    QString currentPaintOpId();

    ///fill the cutoutOverlay rect with the cotent of an image, used to get the image back when selecting a preset
    ///@param image image that will be used, should be image of an existing preset resource
    void setPresetImage(const QImage& image);

    void resizeEvent(QResizeEvent* ) override;

    bool detached() const;

    void updateViewSettings();

    void currentPresetChanged(KisPaintOpPresetSP  preset);

    KisPresetSaveWidget * saveDialog;

    // toggle the state when we are creating a brush from scratch
    void setCreatingBrushFromScratch(bool enable);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

public Q_SLOTS:
    void switchDetached(bool show = true);
    void resourceSelected(KoResource* resource);
    void updateThemedIcons();


    void slotUpdatePresetSettings();
    void slotUpdateLodAvailability();
    void slotRenameBrushActivated();
    void slotRenameBrushDeactivated();
    void slotSaveRenameCurrentBrush();
    void slotCreateNewBrushPresetEngine();

Q_SIGNALS:
    void savePresetClicked();
    void saveBrushPreset();
    void defaultPresetClicked();
    void paintopActivated(const QString& presetName);
    void signalResourceSelected(KoResource* resource);
    void reloadPresetClicked();
    void dirtyPresetToggled(bool value);
    void eraserBrushSizeToggled(bool value);
    void eraserBrushOpacityToggled(bool value);
    void brushEditorShown();
    void createPresetFromScratch(const QString& paintOpName);

private Q_SLOTS:
    void slotSwitchScratchpad(bool visible);
    void slotResourceChanged(int key, const QVariant &value);
    void slotLodAvailabilityChanged(bool value);
    void slotLodThresholdChanged(qreal value);
    void slotSwitchShowEditor(bool visible);
    void slotUpdatePaintOpFilter();
    void slotSwitchShowPresets(bool visible);
    void slotSaveBrushPreset();
    void slotSaveNewBrushPreset();

    /// we do not delete brushe presets, but blacklist them so they disappear from the interface
    void slotBlackListCurrentPreset();

private:

    struct Private;
    Private * const m_d;
    QString current_paintOpId;
    QList<KisPaintOpInfo> sortedBrushEnginesList;


    QMenu * newPresetBrushEnginesMenu;
    QList<KisAction*> newBrushEngineOptions;

    QMenu * brushConfigurationMenu;

    QAction* brushConfigAction;


    void toggleBrushRenameUIActive(bool isRenaming);
};

#endif
