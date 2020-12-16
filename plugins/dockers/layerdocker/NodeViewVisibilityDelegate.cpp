/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "NodeViewVisibilityDelegate.h"

#include "kis_node_view_color_scheme.h"


NodeViewVisibilityDelegate::NodeViewVisibilityDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

NodeViewVisibilityDelegate::~NodeViewVisibilityDelegate()
{
}

void NodeViewVisibilityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(index);
}

QSize NodeViewVisibilityDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    KisNodeViewColorScheme scm;
    return QSize(option.rect.width(), scm.rowHeight());
}
