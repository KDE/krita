/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _KIS_ANIMATION_CURVE_CHANNEL_LIST_MODEL_H
#define _KIS_ANIMATION_CURVE_CHANNEL_LIST_MODEL_H

#include <QAbstractItemModel>

#include "kis_types.h"

class KisAnimationCurvesModel;
class KisDummiesFacadeBase;

class KisAnimationCurveChannelListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    KisAnimationCurveChannelListModel(KisAnimationCurvesModel *curvesModel, QObject *parent);
    ~KisAnimationCurveChannelListModel();

    void setDummiesFacade(KisDummiesFacadeBase *facade);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    enum ItemDataRole
    {
        CurveColorRole = Qt::UserRole,
        CurveVisibleRole
    };

public Q_SLOTS:
    void selectedNodesChanged(const KisNodeList &nodes);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
