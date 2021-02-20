/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoLineStyleItemDelegate_p.h"

#include <QPen>
#include <QPainter>

KoLineStyleItemDelegate::KoLineStyleItemDelegate(QObject * parent)
    : QAbstractItemDelegate(parent)
{
}

void KoLineStyleItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QPen pen = index.data(Qt::DecorationRole).value<QPen>();
    pen.setBrush(option.palette.text()); // use the view-specific palette; the model hardcodes this to black
    painter->setPen(pen);
    painter->drawLine(option.rect.left(), option.rect.center().y(), option.rect.right(), option.rect.center().y());

    painter->restore();
}

QSize KoLineStyleItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(100, 15);
}
