/*
 *  kis_cmb_imagetype.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef KIS_CMB_IDLIST_H_
#define KIS_CMB_IDLIST_H_

#include "qcombobox.h"

#include "KoID.h"
#include <krita_export.h>

/**
 * A combobox that is associated with a list of KoID's. The
 * descriptive (i18n'ed) text is displayed, but the various
 * signals return a KoID.
 */
class KRITAUI_EXPORT KisCmbIDList : public QComboBox
{
    typedef QComboBox super;

    Q_OBJECT

public:

    KisCmbIDList(QWidget * parent = 0, const char * name = 0 );
    virtual ~KisCmbIDList();


public:
    void setIDList(const QList<KoID> & list);
    void setCurrent(const KoID id);
    void setCurrent(const QString & s);

    KoID currentItem() const;

signals:

    void activated(const KoID &);
    void highlighted(const KoID &);

private slots:

    void slotIDActivated(int i);
    void slotIDHighlighted(int i);

private:
    // Prevent the deprecated Qt3 method being called. Use setCurrent instead.
    void setCurrentText(const QString & s);

    QList<KoID> m_list;

};
#endif
