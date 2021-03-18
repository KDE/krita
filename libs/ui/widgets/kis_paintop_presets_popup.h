/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include "dialogs/KisDlgSavePreset.h"
#include "widgets/kis_paintop_presets_popup.h"
#include "kis_favorite_resource_manager.h"
#include "KoDialog.h"

class QString;
class KisCanvasResourceProvider;
class KoResource;

/**
 * Popup widget for presets with built-in functionality
 * for adding and removing presets.
 */
class KisPaintOpPresetsEditor : public QWidget
{
    Q_OBJECT

public:

    KisPaintOpPresetsEditor(KisCanvasResourceProvider * resourceProvider,
                           KisFavoriteResourceManager* favoriteResourceManager,
                           KisPresetSaveWidget* savePresetWidget,
                           QWidget * parent = 0);

    ~KisPaintOpPresetsEditor() override;

    void setPaintOpSettingsWidget(QWidget * widget);


    ///Image for preset preview
    ///@return image cut out from the scratchpad
    QImage cutOutOverlay();

    void setPaintOpList(const QList<KisPaintOpFactory*>& list);

    void setCurrentPaintOpId(const QString & paintOpId);

    /// returns the internal ID for the paint op (brush engine)
    QString currentPaintOpId();

    ///fill the cutoutOverlay rect with the content of an image, used to get the image back when selecting a preset
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
    void resourceSelected(KoResourceSP resource);
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
    void signalResourceSelected(KoResourceSP resource);
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


    void toggleBrushRenameUIActive(bool isRenaming);
};

class KisPaintOpPresetsEditorDialog : public KoDialog
{
    Q_OBJECT

public:
    KisPaintOpPresetsEditorDialog(KisCanvasResourceProvider * resourceProvider,
                           KisFavoriteResourceManager* favoriteResourceManager,
                           KisPresetSaveWidget* savePresetWidget,
                           QWidget * parent = 0);



    KisPaintOpPresetsEditor*  m_popupWidget {0};
    ~KisPaintOpPresetsEditorDialog() override;

    KisPaintOpPresetsEditor* editorWidget() { return m_popupWidget; }

};



#endif
