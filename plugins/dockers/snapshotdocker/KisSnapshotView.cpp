/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
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

