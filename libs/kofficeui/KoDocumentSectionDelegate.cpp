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

#include <QtDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QStyleOptionViewItem>
#include "KoDocumentSectionModel.h"
#include "KoDocumentSectionToolTip.h"
#include "KoDocumentSectionView.h"
#include "KoDocumentSectionDelegate.h"

class KoDocumentSectionDelegate::Private
{
    public:
        KoDocumentSectionView *view;
        QPointer<QWidget> edit;
        KoDocumentSectionToolTip tip;
        static const int margin = 1;
        Private(): view( 0 ), edit( 0 ) { }
};

KoDocumentSectionDelegate::KoDocumentSectionDelegate( KoDocumentSectionView *view, QObject *parent )
    : super( parent )
    , d( new Private )
{
    d->view = view;
    view->setItemDelegate( this );
    QApplication::instance()->installEventFilter( this );
}

KoDocumentSectionDelegate::~KoDocumentSectionDelegate()
{
    delete d;
}

QSize KoDocumentSectionDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    switch( d->view->displayMode() )
    {
        case View::ThumbnailMode:
        {
            const int height = thumbnailHeight( option, index ) + textBoxHeight( option ) + d->margin * 2;
            return QSize( availableWidth( index ), height );
        }
        case View::DetailedMode:
            return QSize( option.rect.width(),
                textBoxHeight( option ) + option.decorationSize.height() + d->margin );
        case View::MinimalMode:
            return QSize( option.rect.width(), textBoxHeight( option ) );
        default: return option.rect.size(); //gcc--
    }
}

void KoDocumentSectionDelegate::paint( QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &index ) const
{
    p->save();
    {
        QStyleOptionViewItem option = getOptions( o, index );

        p->setFont( option.font );

        if( option.state & QStyle::State_Selected )
            p->fillRect( option.rect, option.palette.highlight() );

        drawText( p, option, index );
        drawIcons( p, option, index );
        drawThumbnail( p, option, index );
        drawDecoration( p, option, index );
    }
    p->restore();
}

bool KoDocumentSectionDelegate::editorEvent( QEvent *e, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index )
{
    if( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick )
    {
        QMouseEvent *me = static_cast<QMouseEvent*>( e );
        if( me->button() != Qt::LeftButton )
        {
            d->view->setCurrentIndex( index );
            return false;
        }

        const QRect ir = iconsRect( option, index ).translated( option.rect.topLeft() ),
                    tr = textRect( option, index ).translated( option.rect.topLeft() );

        if( ir.isValid() && ir.contains( me->pos() ) )
        {
            const int iconWidth = option.decorationSize.width();
            int x = me->pos().x() - ir.left();
            if( x % ( iconWidth + d->margin ) < iconWidth ) //it's on an icon, not a margin
            {
                Model::PropertyList lp = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
                int p = -1;
                for( int i = 0, n = lp.count(); i < n; ++i )
                {
                    if( lp[i].isMutable )
                        x -= iconWidth + d->margin;
                    p += 1;
                    if( x < 0 )
                        break;
                }
                lp[p].state = !lp[p].state.toBool();
                model->setData( index, QVariant::fromValue( lp ), Model::PropertiesRole );
            }
            return true;
        }

        else if( tr.isValid() && tr.contains( me->pos() ) && ( option.state & QStyle::State_HasFocus ) )
        {
            d->view->edit( index );
            return true;
        }

        if( !( me->modifiers() & Qt::ControlModifier) && !( me->modifiers() & Qt::ShiftModifier ) )
            d->view->setCurrentIndex( index );
    }

    else if( e->type() == QEvent::ToolTip )
    {
        QHelpEvent *he = static_cast<QHelpEvent*>( e );
        d->tip.showTip( d->view, he->pos(), option, index );
        return true;
    }

    else if( e->type() == QEvent::Leave )
    {
        d->tip.hide();
    }

    return false;
}

QWidget *KoDocumentSectionDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem&, const QModelIndex& ) const
{
    d->edit = new QLineEdit( parent );
    d->edit->installEventFilter( const_cast<KoDocumentSectionDelegate*>( this ) ); //hack?
    return d->edit;
}

void KoDocumentSectionDelegate::setEditorData( QWidget *widget, const QModelIndex &index ) const
{
    QLineEdit *edit = qobject_cast<QLineEdit*>( widget );
    Q_ASSERT( edit );

    edit->setText( index.data( Qt::DisplayRole ).toString() );
}

void KoDocumentSectionDelegate::setModelData( QWidget *widget, QAbstractItemModel *model, const QModelIndex &index ) const
{
    QLineEdit *edit = qobject_cast<QLineEdit*>( widget );
    Q_ASSERT( edit );

    model->setData( index, edit->text(), Qt::DisplayRole );
}

void KoDocumentSectionDelegate::updateEditorGeometry( QWidget *widget, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    widget->setGeometry( textRect( option, index ).translated( option.rect.topLeft() ) );
}


// PROTECTED


bool KoDocumentSectionDelegate::eventFilter( QObject *object, QEvent *event )
{
    if( event->type() == QEvent::MouseButtonPress && d->edit )
    {
        QMouseEvent *me = static_cast<QMouseEvent*>( event );
        if( !QRect( d->edit->mapToGlobal( QPoint() ), d->edit->size() ).contains( me->globalPos() ) )
            emit closeEditor( d->edit );
    }

    QLineEdit *edit = qobject_cast<QLineEdit*>( object );
    if( edit && edit == d->edit && event->type() == QEvent::KeyPress )
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>( event );
        switch( ke->key() )
        {
            case Qt::Key_Escape:
                emit closeEditor( edit );
                return true;
            case Qt::Key_Tab:
                emit commitData( edit );
                emit closeEditor( edit, EditNextItem );
                return true;
            case Qt::Key_Backtab:
                emit commitData( edit );
                emit closeEditor( edit, EditPreviousItem );
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                emit commitData( edit );
                emit closeEditor( edit );
                return true;
            default: break;
        }
    }

    return super::eventFilter( object, event );
}


// PRIVATE


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

int KoDocumentSectionDelegate::thumbnailHeight( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QSize size = index.data( Qt::SizeHintRole ).toSize();
    int width = option.rect.width();
    if( !option.rect.isValid() )
        width = availableWidth( index );
    if( size.width() <= width )
        return size.height();
    else
        return int( width / ( double( size.width() ) / size.height() ) );
}

int KoDocumentSectionDelegate::availableWidth( const QModelIndex &index ) const
    //this is such a HACK
{
    int dis = 0;
    if( d->view->rootIndex().isValid() )
    {
        QModelIndex i = index;
        while( i.isValid() && i != d->view->rootIndex() )
        {
            i = i.parent();
            dis++;
        }
        Q_ASSERT( i.isValid() );
    }
    if( d->view->rootIsDecorated() )
        dis++;
    const int indent = dis * d->view->indentation();
    return d->view->columnWidth( 0 ) - indent;
}

int KoDocumentSectionDelegate::textBoxHeight( const QStyleOptionViewItem &option ) const
{
    return qMax( option.fontMetrics.height(), option.decorationSize.height() );
}

QRect KoDocumentSectionDelegate::textRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    if( d->view->displayMode() == View::ThumbnailMode )
    {
        const QRect r = decorationRect( option, index );
        const int left = r.right() + d->margin;
        return QRect( left, r.top(), option.rect.width() - left, textBoxHeight( option ) );
    }
    else
    {
        static QFont f;
        static int minbearing = 1337 + 666; //can be 0 or negative, 2003 is less likely
        if( minbearing == 2003 || f != option.font )
        {
            f = option.font; //getting your bearings can be expensive, so we cache them
            minbearing = option.fontMetrics.minLeftBearing() + option.fontMetrics.minRightBearing();
        }

        int indent = decorationRect( option, index ).right() + d->margin;

        const int width = ( d->view->displayMode() == View::DetailedMode
                            ? option.rect.width()
                            : iconsRect( option, index ).left() )
                          - indent - d->margin + minbearing;

        return QRect( indent, 0, width, textBoxHeight( option ) );
    }
}

QRect KoDocumentSectionDelegate::iconsRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    if( d->view->displayMode() == View::ThumbnailMode )
        return QRect();

    Model::PropertyList lp = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    int propscount = 0;
    for( int i = 0, n = lp.count(); i < n; ++i )
        if( lp[i].isMutable )
            propscount++;

    const int iconswidth = propscount * option.decorationSize.width() + (propscount - 1) * d->margin;

    const int x = d->view->displayMode() == View::DetailedMode ? thumbnailRect( option, index ).right() + d->margin : option.rect.width() - iconswidth;
    const int y = d->view->displayMode() == View::DetailedMode ? textBoxHeight( option ) + d->margin : 0;

    return QRect( x, y, iconswidth, option.decorationSize.height() );
}

QRect KoDocumentSectionDelegate::thumbnailRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    if( d->view->displayMode() == View::ThumbnailMode )
        return QRect( 0, 0, option.rect.width(), thumbnailHeight( option, index ) );
    else
        return QRect( 0, 0, option.rect.height(), option.rect.height() );
}

QRect KoDocumentSectionDelegate::decorationRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    int width = option.decorationSize.width();
    if( index.data( Qt::DecorationRole ).value<QIcon>().isNull() )
        width = 0;
    switch( d->view->displayMode() )
    {
        case View::ThumbnailMode:
        {
            QFont font = option.font;
            if( index.data( Model::ActiveRole ).toBool() )
                font.setBold( !font.bold() );
            const QFontMetrics metrics( font );
            const int totalwidth = metrics.width( index.data( Qt::DisplayRole ).toString() ) + width + d->margin;
            int left;
            if( totalwidth < option.rect.width() )
                left = ( option.rect.width() - totalwidth ) / 2;
            else
                left = 0;
            return QRect( left, thumbnailRect( option, index ).bottom() + d->margin, width, textBoxHeight( option ) );
        }
        case View::DetailedMode:
        case View::MinimalMode:
        {
            const int left = thumbnailRect( option, index ).right() + d->margin;
            return QRect( left, 0, width, textBoxHeight( option ) );
        }
        default: return QRect(); //gcc--
    }
}

void KoDocumentSectionDelegate::drawText( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QRect r = textRect( option, index ).translated( option.rect.topLeft() );

    p->save();
    {
        p->setClipRect( r );
        p->translate( r.left(), r.top() );
        p->setPen( ( option.state & QStyle::State_Selected )
                   ? option.palette.highlightedText().color()
                   : option.palette.text().color() );

        if( index.data( Model::ActiveRole ).toBool() )
        {
            QFont f = p->font();
            f.setBold( !f.bold() );
            p->setFont( f );
        }

        const QString text = index.data( Qt::DisplayRole ).toString();
        const QString elided = elidedText( p->fontMetrics(), r.width(), Qt::ElideRight, text );
        p->drawText( d->margin, 0, r.width(), r.height(), Qt::AlignLeft | Qt::AlignTop, elided );
    }
    p->restore();
}

void KoDocumentSectionDelegate::drawIcons( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QRect r = iconsRect( option, index ).translated( option.rect.topLeft() );

    p->save();
    {
        p->setClipRect( r );
        p->translate( r.left(), r.top() );
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
    p->restore();
}

void KoDocumentSectionDelegate::drawThumbnail( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QRect r = thumbnailRect( option, index ).translated( option.rect.topLeft() );

    p->save();
    {
        p->setClipRect( r );
        const double myratio = double( r.width() ) / r.height();
        const double thumbratio = index.data( Model::AspectRatioRole ).toDouble();
        const int s = ( myratio > thumbratio ) ? r.height() : r.width();

        const QImage i = index.data( int( Model::BeginThumbnailRole ) + s ).value<QImage>();
        QPoint offset;
        offset.setX( r.width()/2 - i.width()/2 );
        offset.setY( r.height()/2 - i.height()/2 );

        p->drawImage( r.topLeft() + offset, i );
    }
    p->restore();
}

void KoDocumentSectionDelegate::drawDecoration( QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    const QRect r = decorationRect( option, index ).translated( option.rect.topLeft() );

    p->save();
    {
        p->setClipRect( r );
        p->translate( r.topLeft() );
        if( !index.data( Qt::DecorationRole ).value<QIcon>().isNull() )
            p->drawPixmap( 0, 0, index.data( Qt::DecorationRole ).value<QIcon>().pixmap( option.decorationSize ) );
    }
    p->restore();
}



#include "KoDocumentSectionDelegate.moc"
