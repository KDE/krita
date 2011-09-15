/*
 *  widgets/kis_cmb_composite.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef KIS_COMPOSITEOP_WIDGETS_H_
#define KIS_COMPOSITEOP_WIDGETS_H_

#include <krita_export.h>
#include <QComboBox>
#include "kis_categorized_list_view.h"
#include "../kis_composite_ops_model.h"

class KisCompositeOpListModel;

class KRITAUI_EXPORT KisCompositeOpListWidget: public KisCategorizedListView, public KisCategorizedWidgetBase<KisCompositeOpListModel>
{
public:
     KisCompositeOpListWidget(QWidget* parent=0);
    ~KisCompositeOpListWidget();
};


class KRITAUI_EXPORT KisCompositeOpComboBox: public QComboBox, public KisCategorizedWidgetBase<KisCompositeOpListModel>
{
    Q_OBJECT
public:
     KisCompositeOpComboBox(QWidget* parent=0);
    ~KisCompositeOpComboBox();
    
    virtual void hidePopup();
    
private slots:
    void slotCategoryToggled(const QModelIndex& index, bool toggled);
    void slotEntryChecked(const QModelIndex& index);
    
private:
    KisCategorizedListView* m_view;
    bool                    m_allowToHidePopup;
};

#endif // KIS_COMPOSITEOP_WIDGETS_H_
