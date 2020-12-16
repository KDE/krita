/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __NODE_VIEW_VISIBILITY_DELEGATE_H
#define __NODE_VIEW_VISIBILITY_DELEGATE_H

#include <QAbstractItemDelegate>


class NodeViewVisibilityDelegate : public QAbstractItemDelegate
{
public:
    NodeViewVisibilityDelegate(QObject *parent);
    ~NodeViewVisibilityDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
