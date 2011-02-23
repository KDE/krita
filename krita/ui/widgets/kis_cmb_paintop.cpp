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

#include <QStyledItemDelegate>

#include <klocale.h>
#include <kis_debug.h>
#include <kis_paintop_factory.h>
#include "../kis_paint_ops_model.h"
#include <KCategorizedSortFilterProxyModel>
#include <kis_categorized_item_delegate.h>

KisCmbPaintop::KisCmbPaintop(QWidget * parent, const char * name)
        : QListView(parent), m_lastModel(0), m_sortModel(0)
{
    setObjectName(name);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(slotOpActivated(QModelIndex)));
    setItemDelegate(new KisCategorizedItemDelegate(new QStyledItemDelegate));
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

    delete m_lastModel;
    m_lastModel = model;
}

const QString& KisCmbPaintop::itemAt(int idx) const
{
    return m_lastModel->itemAt(m_sortModel->mapToSource(m_sortModel->index(idx, 0)));
}

const QString& KisCmbPaintop::currentItem() const
{
    return itemAt(currentIndex().row());
}

void KisCmbPaintop::setCurrent(const KisPaintOpFactory* op)
{
    QModelIndex index = m_sortModel->mapFromSource(m_lastModel->indexOf(op));
    if (index.isValid()) {
        QListView::setCurrentIndex(index);
    }
}

void KisCmbPaintop::setCurrent(const QString & paintOpId)
{
    QModelIndex index = m_sortModel->mapFromSource(m_lastModel->indexOf(paintOpId));
    if (index.isValid()) {
        QListView::setCurrentIndex(index);
    }
}

void KisCmbPaintop::slotOpActivated(const QModelIndex& index)
{
    int i = index.row();
    if (i >= m_lastModel->rowCount()) return;

    emit activated(itemAt(i));
}


#include "kis_cmb_paintop.moc"

