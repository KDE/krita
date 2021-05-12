/*
 *  kis_paintop_box.h - part of KImageShop/Krayon/Krita
 *
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt (boud@valdyas.org)
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTOP_BOX_H_
#define KIS_PAINTOP_BOX_H_

#include <QMap>
#include <QWidget>
#include <QList>

#include <KoResource.h>
#include <KoID.h>
#include <KoInputDevice.h>

#include <kis_types.h>
#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_locked_properties_proxy.h>
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties.h>
#include "kritaui_export.h"
#include "kis_signal_auto_connection.h"
#include "kis_signal_compressor.h"


class QToolButton;
class QString;
class QHBoxLayout;

class KoColorSpace;
class KoCanvasController;

class KisViewManager;
class KisCanvasResourceProvider;
class KisPopupButton;
class KisIconWidget;
class KisToolOptionsPopup;
class KisPaintOpPresetsEditor;
class KisPaintOpPresetsEditorDialog;
class KisPaintOpPresetsChooserPopup;
class KisPaintOpConfigWidget;
class KisCompositeOpComboBox;
class KisWidgetChooser;
class KisFavoriteResourceManager;
class KisAction;
class KisPresetSaveWidget;

/**
 * This widget presents all paintops that a user can paint with.
 * Paintops represent real-world tools or the well-known Shoup
 * computer equivalents that do nothing but change color.
 *
 * To incorporate the dirty preset functionality and locked settings
 * the following slots are added
 *  void slotReloadPreset();
    void slotGuiChangedCurrentPreset();
    void slotSaveLockedOptionToPreset(KisPropertiesConfigurationSP p);
    void slotDropLockedOption(KisPropertiesConfigurationSP p);
    void slotDirtyPresetToggled(bool);
    Every time a value is changed in a preset, the preset is made dirty through the onChange() function.
    For Locked Settings however, a changed Locked Setting will not cause a preset to become dirty. That is
    because it borrows its values from the KisLockedPropertiesServer.
    Hence the dirty state of the Preset is kept consistent before and after a writeConfiguration operation in  most cases.
 * XXX: When we have a lot of paintops, replace the listbox
 * with a table, and for every category a combobox.
 *
 * XXX: instead of text, use pretty pictures.
 */
class KRITAUI_EXPORT KisPaintopBox : public QWidget
{
    Q_OBJECT

    enum {
        ENABLE_PRESETS      = 0x0001,
        DISABLE_PRESETS     = 0x0002,
        ENABLE_COMPOSITEOP  = 0x0004,
        DISABLE_COMPOSITEOP = 0x0008,
        ENABLE_OPACITY      = 0x0010,
        DISABLE_OPACITY     = 0x0020,
        ENABLE_FLOW         = 0x0040,
        DISABLE_FLOW        = 0x0080,
        ENABLE_SIZE         = 0x0100,
        DISABLE_SIZE        = 0x0200,
        ENABLE_PATTERNSIZE  = 0x0400,
        DISABLE_PATTERNSIZE = 0x0800,
        ENABLE_ALL          = 0x5555,
        DISABLE_ALL         = 0xAAAA
    };

public:

    KisPaintopBox(KisViewManager* view, QWidget* parent, const char* name);
    ~KisPaintopBox() override;

    void restoreResource(KoResourceSP resource);
    /**
     * Update the option widgets to the argument ones, removing the currently set widgets.
     */
    void newOptionWidgets(const QList<QPointer<QWidget> > & optionWidgetList);

    KisFavoriteResourceManager *favoriteResourcesManager() { return m_favoriteResourceManager; }

public Q_SLOTS:

    void slotColorSpaceChanged(const KoColorSpace* colorSpace);
    void slotInputDeviceChanged(const KoInputDevice & inputDevice);
    void slotCanvasResourceChangeAttempted(int key, const QVariant &value);
    void slotCanvasResourceChanged(int key, const QVariant& v);
    void resourceSelected(KoResourceSP resource);

    /// This should take care of creating a new brush preset from scratch
    /// It will either load the default brush preset for the engine,
    /// or create a new empty preset if a default preset does not exist
    void slotCreatePresetFromScratch(QString paintop);

private:

    void setCurrentPaintop(const KoID& paintop);
    void setCurrentPaintop(KisPaintOpPresetSP preset);

    KisPaintOpPresetSP defaultPreset(const KoID& paintOp);
    KisPaintOpPresetSP activePreset(const KoID& paintOp);
    void updateCompositeOp(QString compositeOpID);
    void setWidgetState(int flags);
    void setSliderValue(const QString& sliderID, qreal value);
    void setMultiplierSliderValue(const QString& sliderID, qreal value);
    void sliderChanged(int n);
    void findDefaultPresets();

private Q_SLOTS:

    void slotSetupDefaultPreset();
    void slotNodeChanged(const KisNodeSP node);
    void slotToggleEraseMode(bool checked);
    void slotSetCompositeMode(int index);
    void slotSetPaintop(const QString& paintOpId);
    void slotHorizontalMirrorChanged(bool value);
    void slotVerticalMirrorChanged(bool value);
    void slotSlider1Changed();
    void slotSlider2Changed();
    void slotSlider3Changed();
    void slotSlider4Changed();
    void slotToolChanged(KoCanvasController* canvas);
    void slotPreviousFavoritePreset();
    void slotNextFavoritePreset();
    void slotSwitchToPreviousPreset();
    void slotUnsetEraseMode();
    void slotToggleAlphaLockMode(bool);
    void slotDisablePressureMode(bool);

    void slotReloadPreset();
    void slotGuiChangedCurrentPreset();
    void slotSaveLockedOptionToPreset(KisPropertiesConfigurationSP p);
    void slotDropLockedOption(KisPropertiesConfigurationSP p);
    void slotDirtyPresetToggled(bool);
    void slotEraserBrushSizeToggled(bool);
    void slotEraserBrushOpacityToggled(bool);
    void slotUpdateSelectionIcon();

    void slotLockXMirrorToggle(bool);
    void slotLockYMirrorToggle(bool);
    void slotMoveToCenterMirrorX();
    void slotMoveToCenterMirrorY();
    void slotHideDecorationMirrorX(bool);
    void slotHideDecorationMirrorY(bool);

    void slotUpdateOptionsWidgetPopup();

    void togglePresetEditor();

private:
    KisCanvasResourceProvider*          m_resourceProvider {0};
    QHBoxLayout*                        m_layout {0};
    QWidget*                            m_paintopWidget {0};
    KisPaintOpConfigWidget*             m_optionWidget {0};
    KisPopupButton*                     m_toolOptionsPopupButton {0};

    KisPresetSaveWidget*                m_savePresetWidget {0};
    KisIconWidget*                      m_brushEditorPopupButton {0};
    KisIconWidget*                      m_presetSelectorPopupButton {0};
    KisCompositeOpComboBox*             m_cmbCompositeOp {0};
    QToolButton*                        m_eraseModeButton {0};
    QToolButton*                        m_alphaLockButton {0};
    QToolButton*                        m_hMirrorButton {0};
    QToolButton*                        m_vMirrorButton {0};
    QToolButton*                        m_wrapAroundButton {0};
    KisToolOptionsPopup*                m_toolOptionsPopup {0};
    KisPaintOpPresetsEditorDialog*      m_presetsEditor {0};
    KisPaintOpPresetsChooserPopup*      m_presetsChooserPopup {0};
    KisViewManager*                     m_viewManager {0};
    KisPopupButton*                     m_workspaceWidget {0};
    KisWidgetChooser*                   m_sliderChooser[4];
    QMap<KoID, KisPaintOpConfigWidget*> m_paintopOptionWidgets;
    KisFavoriteResourceManager*         m_favoriteResourceManager {0};
    QToolButton*                        m_reloadButton {0};
    KisAction*                          m_eraseAction {0};
    KisAction*                          m_reloadAction {0};
    KisAction*                          m_disablePressureAction {0};

    QString    m_currCompositeOpID;
    KisNodeWSP m_previousNode;

    KisAction* m_hMirrorAction {0};
    KisAction* m_vMirrorAction {0};

    KisAction* hideCanvasDecorationsX {0};
    KisAction* lockActionX {0};
    KisAction* moveToCenterActionX {0};
    KisAction* hideCanvasDecorationsY {0};
    KisAction* lockActionY {0};
    KisAction* moveToCenterActionY {0};


    struct TabletToolID {
        TabletToolID(const KoInputDevice& dev) {
            // Only the eraser is special, and we don't look at Cursor
            pointer = QTabletEvent::Pen;
            if (dev.pointer() == QTabletEvent::Eraser) {
                pointer = QTabletEvent::Eraser;
            }
            uniqueTabletId = dev.uniqueTabletId();
        }

        bool operator == (const TabletToolID& id) const {
            return pointer == id.pointer;
        }

        bool operator < (const TabletToolID& id) const {
            return pointer < id.pointer;
        }

        QTabletEvent::PointerType  pointer;
        qint64 uniqueTabletId;
    };

    struct TabletToolData {
        KoID               paintOpID;
        KisPaintOpPresetSP preset;
    };

    typedef QMap<TabletToolID, TabletToolData> TabletToolMap;
    typedef QMap<KoID, KisPaintOpPresetSP>     PaintOpPresetMap;

    TabletToolMap    m_tabletToolMap;
    PaintOpPresetMap m_paintOpPresetMap;
    TabletToolID     m_currTabletToolID {KoInputDevice::invalid()};
    bool             m_presetsEnabled {true};
    bool             m_blockUpdate {false};
    bool             m_dirtyPresetsEnabled {false};
    bool             m_eraserBrushSizeEnabled {false};
    bool             m_eraserBrushOpacityEnabled {false};

    KisSignalAutoConnectionsStore m_presetConnections;

    QString m_eraserName;
    QString m_defaultPresetName;
};

#endif //KIS_PAINTOP_BOX_H_
