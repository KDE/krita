/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_BRUSHENGINE_SELECTOR_H
#define KIS_BRUSHENGINE_SELECTOR_H

#include <QMap>
#include <QList>
#include <QHash>
#include <QWidget>

#include <kpagedialog.h>

#include <KoID.h>
#include <KoInputDevice.h>

#include <kis_types.h>
#include <kis_paintop_settings.h>

class QListWidget;
class KoColorSpace;
class KisCanvasResourceProvider;
class KisPaintOpSettingsWidget;
class KisPaintOpPresetsChooserPopup;
class KisView2;
class KisPaintOpPresetsPopup;
class KoResource;

class KisBrushEngineSelector : public QWidget
{

    Q_OBJECT
    Q_DISABLE_COPY(KisBrushEngineSelector)

public:

    explicit KisBrushEngineSelector(KisView2 * view, QWidget *parent = 0);
    ~KisBrushEngineSelector();

    KisPaintOpPresetSP paintOpPresetSP(KoID * = 0);
    const KoID & currentPaintop();
    void setCurrentPaintop(const KoID & paintop);
    QPixmap paintopPixmap(const KoID & paintop);

signals:

    void signalPaintopChanged(KisPaintOpPresetSP paintop);

protected:

    void contextMenuEvent(QContextMenuEvent *);

public slots:

    void switchDetached();
    void slotInputDeviceChanged(const KoInputDevice & inputDevice);
    void slotCurrentNodeChanged(KisNodeSP node);
    void colorSpaceChanged(const KoColorSpace *cs);

private slots:

    void updatePaintops();
    void resourceSelected(KoResource* resource);
    void slotItemSelected(int index);
    void slotSaveActivePreset();
    void slotUpdatePreset();
    void slotSetupDefaultPreset();

private:

    KoID defaultPaintop(const KoInputDevice & inputDevice);
    KisPaintOpPresetSP activePreset(const KoID & paintop, const KoInputDevice & inputDevice);


    QListWidget* m_cmbPaintops;
    const KoColorSpace* m_colorspace;
    KisView2* m_view;
    KisCanvasResourceProvider *m_resourceProvider;
    KisPaintOpPresetsChooserPopup* m_presetsChooserPopup;
    KisPaintOpSettingsWidget* m_optionWidget;
    KisPaintOpPresetsPopup* m_presetsPopup;
    QMap<KoID, KisPaintOpSettingsWidget*> m_paintopOptionWidgets;
    QList<KoID> m_paintops;
    QList<KoID> m_displayedOps;
    KisPaintOpPresetSP m_activePreset;
    KisNodeSP m_previousNode;
    typedef QHash<KoInputDevice, KoID> InputDevicePaintopMap;
    InputDevicePaintopMap m_currentID;

    typedef QHash<QString, KisPaintOpPresetSP> PresetMap;
    typedef QHash<KoInputDevice, PresetMap > InputDevicePresetsMap;
    InputDevicePresetsMap m_inputDevicePresets;

    bool m_detached;
};

#endif // KIS_BRUSHENGINE_SELECTOR_H
