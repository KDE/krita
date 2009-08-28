/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "LineStyleItemDelegate.h"

#include <QPen>
#include <QPainter>
#include <QAbstractListModel>
#include <QAbstractItemDelegate>

LineStyleItemDelegate::LineStyleItemDelegate( QObject * parent )
    : QAbstractItemDelegate( parent )
{
}

void LineStyleItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    painter->save();

    if( option.state & QStyle::State_Selected )
        painter->fillRect( option.rect, option.palette.highlight() );

    QPen pen = index.data( Qt::DecorationRole ).value<QPen>();

    painter->setPen( pen );
    painter->drawLine( option.rect.left(), option.rect.center().y(), option.rect.right(), option.rect.center().y() );

    painter->restore();
}

QSize LineStyleItemDelegate::sizeHint( const QStyleOptionViewItem &, const QModelIndex & ) const
{
    return QSize( 100, 15 );
}
