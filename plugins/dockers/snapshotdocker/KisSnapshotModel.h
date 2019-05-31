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

#ifndef KIS_SNAPSHOT_MODEL_H_
#define KIS_SNAPSHOT_MODEL_H_

#include <QAbstractListModel>
#include <QScopedPointer>
#include <QPointer>

#include <kis_canvas2.h>

class KisSnapshotModel : public QAbstractListModel
{
public:
    KisSnapshotModel();
    ~KisSnapshotModel() override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void setCanvas(QPointer<KisCanvas2> canvas);

public Q_SLOTS:
    bool slotCreateSnapshot();
    bool slotRemoveActivatedSnapshot();
    bool slotSwitchToActivatedSnapshot(const QModelIndex &index);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KIS_SNAPSHOT_MODEL_H_
