/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoResourceItemDelegate.h"

#include <resources/KoAbstractGradient.h>
#include <resources/KoColorSet.h>
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
    KoColorSet * palette = dynamic_cast<KoColorSet*>( resource );
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
    else if (palette) {
        QImage thumbnail = index.data( Qt::DecorationRole ).value<QImage>();
        painter->setRenderHint(QPainter::SmoothPixmapTransform, thumbnail.width() > innerRect.width() || thumbnail.height() > innerRect.height());
        painter->drawImage(innerRect, thumbnail);
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
