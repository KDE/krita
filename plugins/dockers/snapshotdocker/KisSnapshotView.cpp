/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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

#include "KisSnapshotView.h"
#include "KisSnapshotModel.h"

#include <kis_assert.h>

struct KisSnapshotView::Private
{
    KisSnapshotModel *model;
};

KisSnapshotView::KisSnapshotView()
    : QListView()
    , m_d(new Private)
{
}

KisSnapshotView::~KisSnapshotView()
{
}

void KisSnapshotView::setModel(QAbstractItemModel *model)
{
    KisSnapshotModel *snapshotModel = dynamic_cast<KisSnapshotModel *>(model);
    if (snapshotModel) {
        QListView::setModel(model);
        m_d->model = snapshotModel;
    }
}

void KisSnapshotView::slotSwitchToSelectedSnapshot()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->model);
    QModelIndexList indexes = selectedIndexes();
    if (indexes.size() != 1) {
        return;
    }
    m_d->model->slotSwitchToSnapshot(indexes[0]);
}

void KisSnapshotView::slotRemoveSelectedSnapshot()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->model);
    QModelIndexList indexes = selectedIndexes();
    Q_FOREACH (QModelIndex index, indexes) {
        m_d->model->slotRemoveSnapshot(index);
    }
}

