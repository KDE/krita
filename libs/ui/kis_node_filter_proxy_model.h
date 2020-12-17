/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    void setAcceptedLabels(const QSet<int> &value);
    void setTextFilter(const QString &text);

    KisNodeSP nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(KisNodeSP node) const;

    void unsetDummiesFacade();

Q_SIGNALS:
    void sigBeforeBeginRemoveRows(const QModelIndex &parent, int start, int end);

public Q_SLOTS:
    void setActiveNode(KisNodeSP node);

private Q_SLOTS:
    void slotUpdateCurrentNodeFilter();
    void slotBeforeBeginRemoveRows(const QModelIndex &parent, int start, int end);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_NODE_FILTER_PROXY_MODEL_H */
