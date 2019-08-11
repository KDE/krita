/*
 *  kis_cmb_imagetype.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include <QComboBox>
#include <KoID.h>
#include <kritaui_export.h>

/**
 * A combobox that is associated with a list of KoID's. The
 * descriptive (i18n'ed) text is displayed, but the various
 * signals return a KoID.
 */
class KRITAUI_EXPORT KisCmbIDList: public QComboBox
{
    Q_OBJECT

public:
    KisCmbIDList(QWidget * parent = 0, const char * name = 0);
    ~KisCmbIDList() override;


public:
    /**
     * @brief setIDList clears the combobox and sorts
     *    the given list by user-visible name and then adds
     *    the items to the combobox
     * @param list the (unsorted) list of KoID's
     * @param sorted if true, the id's will be sorted by name
     */
    void setIDList(const QList<KoID> & list, bool sorted = true);
    void setCurrent(const KoID id);
    void setCurrent(const QString & s);

    KoID currentItem() const;

Q_SIGNALS:
    void activated(const KoID &);
    void highlighted(const KoID &);

private Q_SLOTS:
    void slotIDActivated(int i);
    void slotIDHighlighted(int i);

private:
    QList<KoID> m_list;
};
#endif
