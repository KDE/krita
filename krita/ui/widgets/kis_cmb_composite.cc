/*
 *  kis_cmb_composite.cc - part of KImageShop/Krayon/Krita
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

#include "widgets/kis_cmb_composite.h"

#include <QItemDelegate>
#include <QMouseEvent>

#include <klocale.h>
#include <kis_debug.h>
#include <KoCompositeOp.h>
#include <KCategorizedSortFilterProxyModel>
#include <kis_categorized_item_delegate.h>

#include "kis_categorized_list_model.h"
#include "../kis_composite_ops_model.h"

//////////////////////////////////////////////////////////////////////////////////////////
// ---- KisCompositeOpListView -------------------------------------------------------- //

KisCompositeOpListView::KisCompositeOpListView(QWidget* parent):
    QListView(parent)
{
    connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(slotIndexChanged(const QModelIndex&)));
}

void KisCompositeOpListView::slotIndexChanged(const QModelIndex& index)
{
    if(model()->data(index, IsHeaderRole).toBool()) {
        bool expanded = model()->data(index, ExpandCategoryRole).toBool();
        int beg       = model()->data(index, CategoryBeginRole).toInt();
        int end       = model()->data(index, CategoryEndRole).toInt();
        
        model()->setData(index, !expanded, ExpandCategoryRole);
        
        for(; beg!=end; ++beg)
            setRowHidden(beg, expanded);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// ---- KisCompositeOpListWidget ------------------------------------------------------ //

KisCompositeOpListWidget::KisCompositeOpListWidget(QWidget* parent):
    KisCompositeOpListView(parent)
{
    m_model    = new KisCompositeOpListModel();
    m_delegate = new KisCategorizedItemDelegate2(m_model);
    m_model->fill(KoCompositeOpRegistry::instance().getCompositeOps());
    
    setModel(m_model);
    setItemDelegate(m_delegate);
}


KisCompositeOpListWidget::~KisCompositeOpListWidget()
{
    delete m_model;
    delete m_delegate;
}


//////////////////////////////////////////////////////////////////////////////////////////
// ---- KisCompositeOpComboBox -------------------------------------------------------- //

KisCompositeOpComboBox::KisCompositeOpComboBox(QWidget* parent):
    QComboBox(parent)
{
    m_view     = new KisCompositeOpListView();
    m_model    = new KisCompositeOpListModel();
    m_delegate = new KisCategorizedItemDelegate2(m_model);
    m_model->fill(KoCompositeOpRegistry::instance().getCompositeOps());
    
    setMaxVisibleItems(100);
    setSizeAdjustPolicy(AdjustToContents);
    
    setView(m_view);
    setModel(m_model);
    setItemDelegate(m_delegate);
}

KisCompositeOpComboBox::~KisCompositeOpComboBox()
{
    delete m_view;
    delete m_model;
    delete m_delegate;
}

#include "kis_cmb_composite.moc"

