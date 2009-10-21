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

#include <klocale.h>
#include <kis_debug.h>
#include <KoCompositeOp.h>
#include "../kis_composite_ops_model.h"
#include <KCategorizedSortFilterProxyModel>
#include <kis_categorized_item_delegate.h>

KisCmbComposite::KisCmbComposite(QWidget * parent, const char * name)
        : KComboBox(parent), m_lastModel(0), m_sortModel(0)
{
    setObjectName(name);
    setEditable(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotOpActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotOpHighlighted(int)));
    setItemDelegate(new KisCategorizedItemDelegate(new QItemDelegate));
    m_sortModel = new KCategorizedSortFilterProxyModel;
    m_sortModel->setSortRole(KisCompositeOpsModel::CompositeOpSortRole);
    m_sortModel->setCategorizedModel(true);
    setModel(m_sortModel);
}

KisCmbComposite::~KisCmbComposite()
{
}

void KisCmbComposite::setCompositeOpList(const QList<KoCompositeOp*> & list)
{
    KisCompositeOpsModel* model = new KisCompositeOpsModel(list);
    m_sortModel->setSourceModel(model);
    m_sortModel->sort(0);

    delete m_lastModel;
    m_lastModel = model;
}

const QString& KisCmbComposite::itemAt(int idx) const
{
    return m_lastModel->itemAt(m_sortModel->mapToSource(m_sortModel->index(idx, 0)));
}

const QString& KisCmbComposite::currentItem() const
{
    return itemAt(currentIndex());
}

void KisCmbComposite::setCurrent(const KoCompositeOp* op)
{
    QModelIndex index = m_sortModel->mapFromSource(m_lastModel->indexOf(op));
    if (index.isValid()) {
        KComboBox::setCurrentIndex(index.row());
    }
}

void KisCmbComposite::setCurrent(const QString & s)
{
    QModelIndex index = m_sortModel->mapFromSource(m_lastModel->indexOf(s));
    if (index.isValid()) {
        KComboBox::setCurrentIndex(index.row());
    }
}

void KisCmbComposite::slotOpActivated(int i)
{
    if (i >= m_lastModel->rowCount()) return;

    emit activated(itemAt(i));
}

void KisCmbComposite::slotOpHighlighted(int i)
{
    if (i >= m_lastModel->rowCount()) return;

    emit activated(itemAt(i));
}


#include "kis_cmb_composite.moc"

