/*
 *  kis_cmb_composite.cc - part of KImageShop/Krayon/Krita
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

#include "kis_cmb_composite.h"

#include <KoCompositeOp.h>
#include <KoCompositeOpRegistry.h>
#include "kis_composite_ops_model.h"
#include "kis_categorized_item_delegate.h"


//////////////////////////////////////////////////////////////////////////////////////////
// ---- KisCompositeOpListWidget ------------------------------------------------------ //

KisCompositeOpListWidget::KisCompositeOpListWidget(QWidget* parent):
    KisCategorizedListView(false, parent),
    m_model(new KisSortedCompositeOpListModel(this))
{
    setModel(m_model);
    setItemDelegate(new KisCategorizedItemDelegate(true, this));
}

KisCompositeOpListWidget::~KisCompositeOpListWidget()
{
}

KoID KisCompositeOpListWidget::selectedCompositeOp() const {
    KoID op;

    if (m_model->entryAt(op, currentIndex())) {
        return op;
    }

    return KoCompositeOpRegistry::instance().getDefaultCompositeOp();
}

//////////////////////////////////////////////////////////////////////////////////////////
// ---- KisCompositeOpComboBox -------------------------------------------------------- //

KisCompositeOpComboBox::KisCompositeOpComboBox(QWidget* parent):
    QComboBox(parent),
    m_model(new KisSortedCompositeOpListModel(this)),
    m_allowToHidePopup(true)
{
    m_view = new KisCategorizedListView(true);

    setMaxVisibleItems(100);
    setSizeAdjustPolicy(AdjustToContents);
    m_view->setResizeMode(QListView::Adjust);

    setModel(m_model);
    setView(m_view);
    setItemDelegate(new KisCategorizedItemDelegate(true, this));

    connect(m_view, SIGNAL(sigCategoryToggled(const QModelIndex&, bool)), SLOT(slotCategoryToggled(const QModelIndex&, bool)));
    connect(m_view, SIGNAL(sigEntryChecked(const QModelIndex&)), SLOT(slotEntryChecked(const QModelIndex&)));
}

KisCompositeOpComboBox::~KisCompositeOpComboBox()
{
    delete m_view;
}

void KisCompositeOpComboBox::validate(const KoColorSpace *cs) {
    m_model->validate(cs);
}

void KisCompositeOpComboBox::selectCompositeOp(const KoID &op) {
    QModelIndex index = m_model->indexOf(op);
    setCurrentIndex(index.row());
}

KoID KisCompositeOpComboBox::selectedCompositeOp() const {
    KoID op;

    if (m_model->entryAt(op, m_model->index(currentIndex(), 0))) {
        return op;
    }

    return KoCompositeOpRegistry::instance().getDefaultCompositeOp();
}

void KisCompositeOpComboBox::slotCategoryToggled(const QModelIndex& index, bool toggled)
{
    Q_UNUSED(index);
    Q_UNUSED(toggled);

    //NOTE: this will (should) fit the size of the
    //      popup widget to the view
    //      don't know if this is expected behaviour
    //      on all supported platforms.
    //      Thre is nothing written about this in the docs.
    showPopup();
}

void KisCompositeOpComboBox::slotEntryChecked(const QModelIndex& index)
{
    Q_UNUSED(index);
    m_allowToHidePopup = false;
}

void KisCompositeOpComboBox::hidePopup()
{
    if(m_allowToHidePopup) { QComboBox::hidePopup(); }
    else                   { QComboBox::showPopup(); }

    m_allowToHidePopup = true;
}
