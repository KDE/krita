/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVE_CHANNEL_LIST_DELEGATE_H
#define _KIS_ANIMATION_CURVE_CHANNEL_LIST_DELEGATE_H

#include <qstyleditemdelegate.h>

class KisAnimCurvesChannelDelegate : public QStyledItemDelegate
{
public:
    KisAnimCurvesChannelDelegate(QObject *parent);

    QSize sizeHint(const QStyleOptionViewItem &styleOption, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintNodeBackground( const QStyleOptionViewItem &option, QPainter *painter, const QColor& nodeColor) const;
    void soloChannelVisibility( QAbstractItemModel *model,  const QModelIndex &index );
    void showAllChannels(QAbstractItemModel *model , const QModelIndex &nodeIndex);
};

#endif
