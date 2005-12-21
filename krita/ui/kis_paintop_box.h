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

#include <qcombobox.h>
#include <qvaluelist.h>

class KisView;
class KisID;
class QString;
class KisColorSpace;

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
class KisPaintopBox : public QComboBox {

    Q_OBJECT

    typedef QComboBox super;

public:
    KisPaintopBox (KisView * view,  QWidget * parent, const char * name = 0);

    ~KisPaintopBox();

public slots:

    void addItem(const KisID & paintop, const QString & category = "");

signals:

    void selected(const KisID & id);

private slots:

    void slotItemSelected(int index);
    void colorSpaceChanged(KisColorSpace *cs);

private:
    KisView * m_view;

    QValueList<KisID> * m_paintops;
    QValueList<KisID> * m_displayedOps;
    KisID m_currentID;
};



#endif //KIS_PAINTOP_BOX_H_

