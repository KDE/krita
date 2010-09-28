/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "widgets/kis_cmb_paintop.h"

#include <QItemDelegate>

#include <klocale.h>
#include <kis_debug.h>
#include <kis_paintop_factory.h>
#include "../kis_paint_ops_model.h"
#include <KCategorizedSortFilterProxyModel>
#include <kis_categorized_item_delegate.h>

KisCmbPaintop::KisCmbPaintop(QWidget * parent, const char * name)
        : KComboBox(parent), m_lastModel(0), m_sortModel(0)
{
    setObjectName(name);
    setEditable(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotOpActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotOpHighlighted(int)));
    setItemDelegate(new KisCategorizedItemDelegate(new QItemDelegate));
    m_sortModel = new KCategorizedSortFilterProxyModel;
    m_sortModel->setSortRole(KisPaintOpsModel::PaintOpSortRole);
    m_sortModel->setCategorizedModel(true);
    setModel(m_sortModel);
}

KisCmbPaintop::~KisCmbPaintop()
{
}

void KisCmbPaintop::setPaintOpList(const QList<KisPaintOpFactory*> & list)
{
    KisPaintOpsModel* model = new KisPaintOpsModel(list);
    m_sortModel->setSourceModel(model);
    m_sortModel->sort(0);
    setMaxVisibleItems(list.size());

    delete m_lastModel;
    m_lastModel = model;
}

const QString& KisCmbPaintop::itemAt(int idx) const
{
    return m_lastModel->itemAt(m_sortModel->mapToSource(m_sortModel->index(idx, 0)));
}

const QString& KisCmbPaintop::currentItem() const
{
    return itemAt(currentIndex());
}

void KisCmbPaintop::setCurrent(const KisPaintOpFactory* op)
{
    QModelIndex index = m_sortModel->mapFromSource(m_lastModel->indexOf(op));
    if (index.isValid()) {
        KComboBox::setCurrentIndex(index.row());
    }
}

void KisCmbPaintop::setCurrent(const QString & s)
{
    QModelIndex index = m_sortModel->mapFromSource(m_lastModel->indexOf(s));
    if (index.isValid()) {
        KComboBox::setCurrentIndex(index.row());
    }
}

void KisCmbPaintop::slotOpActivated(int i)
{
    if (i >= m_lastModel->rowCount()) return;

    emit activated(itemAt(i));
}

void KisCmbPaintop::slotOpHighlighted(int i)
{
    if (i >= m_lastModel->rowCount()) return;

    emit activated(itemAt(i));
}


#include "kis_cmb_paintop.moc"

