/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_paintop_list_widget.h"

#include <klocale.h>
#include <kis_debug.h>
#include <kis_paintop_factory.h>
#include "../kis_paint_ops_model.h"
#include "../kis_categorized_item_delegate.h"

KisPaintOpListWidget::KisPaintOpListWidget(QWidget* parent, const char* name):
    KisCategorizedListView(false, parent)
{
    setObjectName(name);
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(slotOpActivated(QModelIndex)));

    m_model    = new KisPaintOpListModel();
    m_delegate = new KisCategorizedItemDelegate(false);
    setModel(m_model);
    setItemDelegate(m_delegate);
}

KisPaintOpListWidget::~KisPaintOpListWidget()
{
    delete m_model;
    delete m_delegate;
}

void KisPaintOpListWidget::setPaintOpList(const QList<KisPaintOpFactory*>& list)
{
    m_model->fill(list);
}

QString KisPaintOpListWidget::itemAt(int idx) const
{
    KisPaintOpInfo info;

    if(m_model->entryAt(info, idx))
        return info.id;

    return "";
}

QString KisPaintOpListWidget::currentItem() const
{
    return itemAt(currentIndex().row());
}

void KisPaintOpListWidget::setCurrent(const KisPaintOpFactory* op)
{
    setCurrentIndex(m_model->indexOf(KisPaintOpInfo(op->id())));
}

void KisPaintOpListWidget::setCurrent(const QString& paintOpId)
{
    setCurrentIndex(m_model->indexOf(KisPaintOpInfo(paintOpId)));
}

void KisPaintOpListWidget::slotOpActivated(const QModelIndex& index)
{
    emit activated(itemAt(index.row()));
}
