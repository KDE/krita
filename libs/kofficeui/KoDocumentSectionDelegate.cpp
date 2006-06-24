/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QPainter>
#include "KoDocumentSectionModel.h"
#include "KoDocumentSectionDelegate.h"

class KoDocumentSectionDelegate::Private
{
    public:
        DisplayMode mode;
        static const int margin = 1;
        Private(): mode( DetailedMode ) { }
};

KoDocumentSectionDelegate::KoDocumentSectionDelegate( QObject *parent )
    : super( parent )
    , d( new Private )
{
}

KoDocumentSectionDelegate::~KoDocumentSectionDelegate()
{
    delete d;
}

QSize KoDocumentSectionDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    switch( d->mode )
    {
        case ThumbnailsMode:
            return QSize( option.rect.width(), option.rect.width() + option.fontMetrics.height() );
        case DetailedMode:
            return QSize( option.rect.width(),
                qMax( index.data( Model::ThumbnailRole ).value<QImage>().height(),
                      option.fontMetrics.height() + option.decorationSize.height() ) );
        case MinimalMode:
            return QSize( option.rect.width(), qMax( option.decorationSize.height(), option.fontMetrics.height() ) );
        default: return option.rect.size(); //gcc--
    }
}

void KoDocumentSectionDelegate::paint( QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &index ) const
{
    p->save();
    {
        QStyleOptionViewItem option = getOptions( o, index );

        p->translate( option.rect.topLeft() );
        p->setFont( option.font );

        // p->fillRect( option.rect, option.palette.base() );

        if( index.data( Qt::DecorationRole ).isValid() )
            p->drawPixmap( thumbnailRect( option, index ).right() + 1 , 0,
                index.data( Qt::DecorationRole ).value<QIcon>().pixmap( option.decorationSize ) );

        drawText( p, option, index );
        drawIcons( p, option, index );
        drawThumbnail( p, option, index );
    }
    p->restore();
}

bool KoDocumentSectionDelegate::editorEvent( QEvent *, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex & )
{
    return false;
}

QStyleOptionViewItem KoDocumentSectionDelegate::getOptions( const QStyleOptionViewItem &o, const QModelIndex &index )
{
    QStyleOptionViewItem option = o;
    QVariant v = index.data( Qt::FontRole );
    if( v.isValid() )
    {
        option.font = v.value<QFont>();
        option.fontMetrics = QFontMetrics( option.font );
    }
    v = index.data( Qt::TextAlignmentRole );
    if( v.isValid() )
        option.displayAlignment = QFlag( v.toInt() );
    v = index.data( Qt::TextColorRole );
    if( v.isValid() )
        option.palette.setColor( QPalette::Text, v.value<QColor>() );
    v = index.data( Qt::BackgroundColorRole );
    if( v.isValid() )
        option.palette.setColor( QPalette::Background, v.value<QColor>() );

   return option;
}

QRect KoDocumentSectionDelegate::textRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    static QFont f;
    static int minbearing = 1337 + 666; //can be 0 or negative, 2003 is less likely
    if( minbearing == 2003 || f != option.font )
    {
        f = option.font; //getting your bearings can be expensive, so we cache them
        minbearing = option.fontMetrics.minLeftBearing() + option.fontMetrics.minRightBearing();
    }

    int indent = thumbnailRect( option, index ).right() + d->margin;
    if( index.data( Qt::DecorationRole ).isValid() )
        indent += option.decorationSize.width() + d->margin;

    const int width = ( d->mode == DetailedMode ? option.rect.right() : iconsRect( option, index ).left() ) - indent - d->margin + minbearing;

    return QRect( indent, 0, width, option.fontMetrics.height() );
}

QRect KoDocumentSectionDelegate::iconsRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    Model::PropertyList lp = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    int propscount = 0;
    for( int i = 0, n = lp.count(); i < n; ++i )
        if( lp[i].isMutable )
            propscount++;

    const int iconswidth = propscount * option.decorationSize.width() + (propscount - 1) * d->margin;

    const int x = d->mode == DetailedMode ? thumbnailRect( option, index ).right() + d->margin : option.rect.width() - iconswidth;
    const int y = d->mode == DetailedMode ? option.fontMetrics.height() : 0;

    return QRect( x, y, iconswidth, option.decorationSize.height() );
}

QRect KoDocumentSectionDelegate::thumbnailRect( const QStyleOptionViewItem &option, const QModelIndex & ) const
{
    return QRect( 0, 0, option.rect.height(), option.rect.height() );
}

void KoDocumentSectionDelegate::drawText( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QRect r = textRect( option, index );

    p->translate( r.left(), r.top() );
    {
        p->setPen( ( option.state & QStyle::State_Selected )
                   ? option.palette.highlightedText().color()
                   : option.palette.text().color() );

        const QString text = index.data( Qt::DisplayRole ).toString();
        const QString elided = elidedText( option.fontMetrics, r.width(), Qt::ElideRight, text );
        p->drawText( d->margin, 0, r.width(), r.height(), Qt::AlignLeft | Qt::AlignTop, elided );
    }
    p->translate( -r.left(), -r.top() );
}

void KoDocumentSectionDelegate::drawIcons( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QRect r = iconsRect( option, index );

    p->translate( r.left(), r.top() );
    {
        int x = 0;
        Model::PropertyList lp = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
        for( int i = 0, n = lp.count(); i < n; ++i )
            if( lp[i].isMutable )
            {
                QIcon icon = lp[i].state.toBool() ? lp[i].onIcon : lp[i].offIcon;
                p->drawPixmap( x, 0, icon.pixmap( option.decorationSize ) );
                x += option.decorationSize.width() + d->margin;
            }
    }
    p->translate( -r.left(), -r.top() );
}

void KoDocumentSectionDelegate::drawThumbnail( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QRect r = thumbnailRect( option, index );

    const QImage i = index.data( Model::ThumbnailRole ).value<QImage>();
    QPoint offset;
    offset.setX( r.width()/2 - i.width()/2 );
    offset.setY( r.height()/2 - i.height()/2 );

    p->drawImage( r.topLeft() + offset, i );
}

#include "KoDocumentSectionDelegate.moc"
