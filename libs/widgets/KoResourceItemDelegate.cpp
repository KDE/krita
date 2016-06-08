/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoResourceItemDelegate.h"

#include <resources/KoAbstractGradient.h>
#include <QPainter>

KoResourceItemDelegate::KoResourceItemDelegate( QObject * parent )
    : QAbstractItemDelegate( parent ), m_checkerPainter( 4 )
{
}

void KoResourceItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if( ! index.isValid() )
        return;

    KoResource * resource = static_cast<KoResource*>( index.internalPointer() );
    if (!resource)
        return;

    painter->save();

    if (option.state & QStyle::State_Selected)
        painter->fillRect( option.rect, option.palette.highlight() );

    QRect innerRect = option.rect.adjusted( 2, 1, -2, -1 );

    KoAbstractGradient * gradient = dynamic_cast<KoAbstractGradient*>( resource );
    if (gradient) {
        QGradient * g = gradient->toQGradient();

        QLinearGradient paintGradient;
        paintGradient.setStops( g->stops() );
        paintGradient.setStart( innerRect.topLeft() );
        paintGradient.setFinalStop( innerRect.topRight() );

        m_checkerPainter.paint( *painter, innerRect );
        painter->fillRect( innerRect, QBrush( paintGradient ) );

        delete g;
    }
    else {
        QImage thumbnail = index.data( Qt::DecorationRole ).value<QImage>();

        QSize imageSize = thumbnail.size();

        if(imageSize.height() > innerRect.height() || imageSize.width() > innerRect.width()) {
            qreal scaleW = static_cast<qreal>( innerRect.width() ) / static_cast<qreal>( imageSize.width() );
            qreal scaleH = static_cast<qreal>( innerRect.height() ) / static_cast<qreal>( imageSize.height() );

            qreal scale = qMin( scaleW, scaleH );

            int thumbW = static_cast<int>( imageSize.width() * scale );
            int thumbH = static_cast<int>( imageSize.height() * scale );
            thumbnail = thumbnail.scaled( thumbW, thumbH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
        }
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        if (thumbnail.hasAlphaChannel()) {
            painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        }
        painter->fillRect( innerRect, QBrush(thumbnail) );
    }
    painter->restore();
}

QSize KoResourceItemDelegate::sizeHint( const QStyleOptionViewItem & optionItem, const QModelIndex & ) const
{
    return optionItem.decorationSize;
}
