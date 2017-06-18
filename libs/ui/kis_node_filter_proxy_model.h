/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_NODE_FILTER_PROXY_MODEL_H
#define __KIS_NODE_FILTER_PROXY_MODEL_H

#include <QScopedPointer>
#include <QSortFilterProxyModel>
#include "kis_types.h"
#include "kritaui_export.h"

class KisNodeModel;
class KisNodeDummy;
class KisNodeManager;


class KRITAUI_EXPORT KisNodeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    KisNodeFilterProxyModel(QObject *parent);
    ~KisNodeFilterProxyModel() override;

    void setNodeModel(KisNodeModel *model);

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    void setAcceptedLabels(const QList<int> &value);

    KisNodeSP nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(KisNodeSP node) const;

    void unsetDummiesFacade();

public Q_SLOTS:
    void setActiveNode(KisNodeSP node);

private Q_SLOTS:
    void slotUpdateCurrentNodeFilter();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_NODE_FILTER_PROXY_MODEL_H */
