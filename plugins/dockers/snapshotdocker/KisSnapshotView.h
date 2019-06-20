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
