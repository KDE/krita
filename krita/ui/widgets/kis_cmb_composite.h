/*
 *  widgets/kis_cmb_composite.h - part of KImageShop/Krayon/Krita
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

#ifndef KIS_CMB_COMPOSITE_H_
#define KIS_CMB_COMPOSITE_H_

#include <krita_export.h>
#include "kcombobox.h"

#include <QListView>
#include <QComboBox>
#include <kis_composite_ops_model.h>

class KisCompositeOpListModel;
class QStyledItemDelegate;

class KRITAUI_EXPORT KisCompositeOpListView: public QListView
{
    Q_OBJECT
public:
    KisCompositeOpListView(QWidget* parent=0);
    
signals:
    void sigCategoryToggled(const QModelIndex& index, bool toggled);
    
protected slots:
    void slotIndexChanged(const QModelIndex& index);
};


template<class TModel>
class KRITAUI_EXPORT KisCategorizedWidgetBase
{
public:
    KisCategorizedWidgetBase():
        m_model(0), m_delegate(0) { }
    
    int  indexOf(const KoID& entry)       const { return m_model->indexOf(entry);        }
    bool entryAt(KoID& result, int index) const { return m_model->entryAt(result,index); }
    
    const TModel* getModel() const { return m_model; }
    TModel*       getModel()       { return m_model; }
    
protected:
    TModel*              m_model;
    QStyledItemDelegate* m_delegate;
};


class KRITAUI_EXPORT KisCompositeOpListWidget: public KisCompositeOpListView, public KisCategorizedWidgetBase<KisCompositeOpListModel>
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
    
private slots:
    void slotCategoryToggled(const QModelIndex& index, bool toggled);
    
private:
    KisCompositeOpListView* m_view;
};

#endif
