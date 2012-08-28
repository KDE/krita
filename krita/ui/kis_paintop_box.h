/*
 *  kis_paintop_box.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004-2008 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (C) 2011      Silvio Heinrich <plassy@web.de>
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

#ifndef KIS_PAINTOP_BOX_H_
#define KIS_PAINTOP_BOX_H_

#include <QMap>
#include <QWidget>
#include <QList>

#include <KoID.h>
#include <KoInputDevice.h>

#include <kis_types.h>
#include <kis_paintop_settings.h>

class QToolButton;
class QPushButton;
class QString;
class QHBoxLayout;

class KoColorSpace;
class KoResourceSelector;
class KoResource;
class KoCompositeOp;
class KoCanvasController;

class KisView2;
class KisCanvasResourceProvider;
class KisPopupButton;
class KisPaintOpPresetsPopup;
class KisPaintOpPresetsChooserPopup;
class KisPaintOpSettingsWidget;
class KisPaintOpListWidget;
class KisCompositeOpComboBox;
class KisWidgetChooser;


/**
 * This widget presents all paintops that a user can paint with.
 * Paintops represent real-world tools or the well-known Shoup
 * computer equivalents that do nothing but change color.
 *
 * XXX: When we have a lot of paintops, replace the listbox
 * with a table, and for every category a combobox.
 *
 * XXX: instead of text, use pretty pictures.
 */
class KisPaintopBox : public QWidget
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
        ENABLE_ALL          = 0x5555,
        DISABLE_ALL         = 0xAAAA
    };

public:
    KisPaintopBox(KisView2* view, QWidget* parent, const char* name);
    KisPaintOpPresetSP paintOpPresetSP(KoID * = 0);
    KoID currentPaintop();
    void setCurrentPaintop(const KoID& paintop, KisPaintOpPresetSP preset=0);
    QPixmap paintopPixmap(const KoID& paintop);
    ~KisPaintopBox();

signals:
    void signalPaintopChanged(KisPaintOpPresetSP paintop);

public slots:
    void slotColorSpaceChanged(const KoColorSpace* colorSpace);
    void slotInputDeviceChanged(const KoInputDevice & inputDevice);
    void slotCurrentNodeChanged(KisNodeSP node);
    void slotSaveActivePreset();
    void slotUpdatePreset();
    void slotSetupDefaultPreset();
    void resourceSelected(KoResource* resource);

private:
    KoID defaultPaintOp();
    KisPaintOpPresetSP defaultPreset(const KoID& paintOp);
    KisPaintOpPresetSP activePreset(const KoID& paintOp);
    void updateCompositeOp(QString compositeOpID);
    void updatePaintops(const KoColorSpace* colorSpace);
    void setWidgetState(int flags);
    void setSliderValue(const QString& sliderID, qreal value);
    void sliderChanged(int n);
    
private slots:
    void slotNodeChanged(const KisNodeSP node);
    void slotToggleEraseMode(bool checked);
    void slotSetCompositeMode(int index);
    void slotSetPaintop(const QString& paintOpId);
    void slotSaveToFavouriteBrushes();
    void slotWatchPresetNameLineEdit(const QString& text);
    void slotHorizontalMirrorChanged(bool value);
    void slotVerticalMirrorChanged(bool value);
    void slotSlider1Changed();
    void slotSlider2Changed();
    void slotToolChanged(KoCanvasController* canvas, int toolId);
    void slotOpacityChanged(qreal);
    
private:
    KisCanvasResourceProvider*           m_resourceProvider;
    QHBoxLayout*                         m_layout;
    QWidget*                             m_paintopWidget;
    KisPaintOpSettingsWidget*            m_optionWidget;
    KisPopupButton*                      m_settingsWidget;
    KisPopupButton*                      m_presetWidget;
    KisPopupButton*                      m_brushChooser;
    KisCompositeOpComboBox*              m_cmbCompositeOp;
    QToolButton*                         m_eraseModeButton;
    KisPaintOpPresetsPopup*              m_presetsPopup;
    KisPaintOpPresetsChooserPopup*       m_presetsChooserPopup;
    KisView2*                            m_view;
    QPushButton*                         m_paletteButton;
    KisPopupButton*                      m_workspaceWidget;
    KisWidgetChooser*                    m_sliderChooser[2];
    QMap<KoID,KisPaintOpSettingsWidget*> m_paintopOptionWidgets;

    KisPaintOpPresetSP  m_activePreset;
    QString             m_prevCompositeOpID;
    QString             m_currCompositeOpID;
    KisNodeSP           m_previousNode;
    
    struct TabletToolID
    {
        TabletToolID(const KoInputDevice& dev) {
            uniqueID = dev.uniqueTabletId();
            pointer  = (dev.pointer() == QTabletEvent::UnknownPointer) ? QTabletEvent::Cursor : dev.pointer();
        }
        
        bool operator == (const TabletToolID& id) const {
            return pointer == id.pointer && uniqueID == id.uniqueID;
        }
        
        bool operator < (const TabletToolID& id) const {
            if(uniqueID == id.uniqueID)
                return pointer < id.pointer;
            return uniqueID < id.uniqueID;
        }
        
        QTabletEvent::PointerType  pointer;
        qint64                     uniqueID;
    };
    
    struct TabletToolData
    {
        KoID               paintOpID;
        KisPaintOpPresetSP preset;
    };

    typedef QMap<TabletToolID, TabletToolData> TabletToolMap;
    typedef QMap<KoID, KisPaintOpPresetSP>     PaintOpPresetMap;
    
    TabletToolMap    m_tabletToolMap;
    PaintOpPresetMap m_paintOpPresetMap;
    TabletToolID     m_currTabletToolID;
    bool             m_presetsEnabled;
    bool             m_blockUpdate;
};

#endif //KIS_PAINTOP_BOX_H_
