/*
 *  kis_paintop_box.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include <map>

#include <QWidget>
#include <QComboBox>
#include <QList>
#include <QPixmap>

#include <KoInputDevice.h>

class QString;
class QHBoxLayout;

class KoID;
class KoColorSpace;

class KisView2;
class KisResourceProvider;
class KisPaintOpSettings;

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
class KisPaintopBox : public QWidget {

    Q_OBJECT

    typedef QWidget super;

public:
    KisPaintopBox (KisView2 * view,  QWidget * parent, const char * name = 0);

    ~KisPaintopBox();

signals:

    void selected(const KoID & id, const KisPaintOpSettings *settings);

private slots:

    void addItem(const KoID & paintop, const QString & category = "");

private slots:

    void slotItemSelected(int index);
    void colorSpaceChanged(KoColorSpace *cs);
    void slotInputDeviceChanged(const KoInputDevice & inputDevice);

private:
    QPixmap paintopPixmap(const KoID & paintop);
    void updateOptionWidget();
    const KoID & currentPaintop();
    void setCurrentPaintop(const KoID & paintop);
    KoID defaultPaintop(const KoInputDevice& inputDevice);
    const KisPaintOpSettings *paintopSettings(const KoID & paintop, const KoInputDevice & inputDevice);

private:
    KisResourceProvider *m_resourceProvider;
    QComboBox * m_cmbPaintops;
    QHBoxLayout * m_layout;
    QWidget * m_optionWidget;

    QList<KoID> m_paintops;
    QList<KoID> m_displayedOps;

    typedef std::map<KoInputDevice, KoID> InputDevicePaintopMap;
    InputDevicePaintopMap m_currentID;

    typedef std::map<KoInputDevice, QList<KisPaintOpSettings *> > InputDevicePaintopSettingsMap;
    InputDevicePaintopSettingsMap m_inputDevicePaintopSettings;
};



#endif //KIS_PAINTOP_BOX_H_

