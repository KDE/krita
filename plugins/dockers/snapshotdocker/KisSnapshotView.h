/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SNAPSHOT_VIEW_H_
#define KIS_SNAPSHOT_VIEW_H_

#include <QListView>

class KisSnapshotView : public QListView
{
public:
    KisSnapshotView();
    ~KisSnapshotView() override;

    void setModel(QAbstractItemModel *model) override;

public Q_SLOTS:
    void slotSwitchToSelectedSnapshot();
    void slotRemoveSelectedSnapshot();
private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
