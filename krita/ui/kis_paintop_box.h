/*
 *  kis_paintop_box.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004-2008 Boudewijn Rempt (boud@valdyas.org)
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

#include <QHash>
#include <QWidget>
#include <QList>
#include <QPixmap>

#include <kcombobox.h>

#include <KoInputDevice.h>

#include <kis_types.h>
#include <kis_paintop_settings.h>

class QString;
class QHBoxLayout;

class KoID;
class KoColorSpace;
class KoResourceSelector;

class KisView2;
class KisCanvasResourceProvider;
class KisPresetWidget;
class KisPaintOpPresetsPopup;
class KisPaintOpSettingsWidget;

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

public:
    KisPaintopBox(KisView2 * view,  QWidget * parent, const char * name);

    ~KisPaintopBox();

private slots:

    void addItem(const KoID & paintop, const QString & category = "");

private slots:

    void slotItemSelected(int index);
    void colorSpaceChanged(const KoColorSpace *cs);
    void slotInputDeviceChanged(const KoInputDevice & inputDevice);
    void slotCurrentNodeChanged(KisNodeSP node);
    void slotSaveActivePreset();

private:

    QPixmap paintopPixmap(const KoID & paintop);
    const KoID & currentPaintop();
    void setCurrentPaintop(const KoID & paintop);
    KoID defaultPaintop(const KoInputDevice & inputDevice);
    KisPaintOpPresetSP activePreset(const KoID & paintop, const KoInputDevice & inputDevice);

private:

    KisCanvasResourceProvider *m_resourceProvider;
    QComboBox * m_cmbPaintops;

    QHBoxLayout * m_layout;
    KisPaintOpSettingsWidget * m_optionWidget;
    KisPresetWidget * m_presetWidget;
    KisPaintOpPresetsPopup * m_presetsPopup;
    KisView2 * m_view;

    QList<KoID> m_paintops;
    QList<KoID> m_displayedOps;
    KisPaintOpPresetSP m_activePreset;

    typedef QHash<KoInputDevice, KoID> InputDevicePaintopMap;
    InputDevicePaintopMap m_currentID;

    typedef QHash<KoInputDevice, QHash<QString, KisPaintOpPresetSP> > InputDevicePresetsMap;
    InputDevicePresetsMap m_inputDevicePresets;
};



#endif //KIS_PAINTOP_BOX_H_
