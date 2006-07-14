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
#include <QAbstractItemView>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QStyleOptionViewItem>
#include <QTextDocument>
#include <QToolTip>
#include <QUrl>
#include <klocale.h>
#include "KoDocumentSectionModel.h"
#include "KoDocumentSectionDelegate.h"

class KoDocumentSectionDelegate::ToolTip: public QFrame
{
    typedef QFrame super;

    public:
        static void showTip( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index );
        static void hideTip();

    private:
        ToolTip();
        ~ToolTip();
        void update( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index );
        void updateDocument( const QStyleOptionViewItem &option, const QModelIndex &index );
        void updatePosition( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option );

        QTextDocument m_document;

        static ToolTip *instance;

    public:
        virtual QSize sizeHint() const;

    protected:
        virtual void paintEvent( QPaintEvent *e );
        virtual bool eventFilter( QObject *object, QEvent *event );
};

class KoDocumentSectionDelegate::Private
{
    public:
        QAbstractItemView *view;
        QPointer<QWidget> edit;
        DisplayMode mode;
        static const int margin = 1;
        Private(): view( 0 ), edit( 0 ), mode( DetailedMode ) { }
};

KoDocumentSectionDelegate::KoDocumentSectionDelegate( QAbstractItemView *view, QObject *parent )
    : super( parent )
    , d( new Private )
{
    d->view = view;
    view->setItemDelegate( this );
}

KoDocumentSectionDelegate::~KoDocumentSectionDelegate()
{
    delete d;
}

void KoDocumentSectionDelegate::setDisplayMode( DisplayMode mode )
{
    d->mode = mode;
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

        p->setFont( option.font );

        if( option.state & QStyle::State_Selected )
            p->fillRect( option.rect, option.palette.highlight() );

        if( !index.data( Qt::DecorationRole ).value<QIcon>().isNull() )
            p->drawPixmap( thumbnailRect( option, index ).right() + 1 , option.rect.top(),
                index.data( Qt::DecorationRole ).value<QIcon>().pixmap( option.decorationSize ) );

        drawText( p, option, index );
        drawIcons( p, option, index );
        drawThumbnail( p, option, index );
    }
    p->restore();
}

bool KoDocumentSectionDelegate::editorEvent( QEvent *e, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index )
{
    if( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick )
    {
        QMouseEvent *me = static_cast<QMouseEvent*>( e );
        if( me->button() != Qt::LeftButton )
            return false; //TODO

        if( d->edit )
        {
            emit closeEditor( d->edit );
            qDebug() << "why doesn't this event ever get received??";
        }

        const QRect ir = iconsRect( option, index ), tr = textRect( option, index );

        if( ir.contains( me->pos() ) )
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

        else if( tr.contains( me->pos() ) )
        {
            d->view->edit( index );
            return true;
        }

        if ( !(me->modifiers() & Qt::ControlModifier) && !(me->modifiers() & Qt::ShiftModifier) )
            d->view->setCurrentIndex( index );
    }

    else if( e->type() == QEvent::ToolTip )
    {
        QHelpEvent *he = static_cast<QHelpEvent*>( e );
        ToolTip::showTip( d->view, he->pos(), option, index );
        return true;
    }

    return false;
}

QWidget *KoDocumentSectionDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem&, const QModelIndex& ) const
{
    d->edit = new QLineEdit( parent );
    d->edit->installEventFilter( const_cast<KoDocumentSectionDelegate*>( this ) );
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
    widget->setGeometry( textRect( option, index ) );
}


// PROTECTED


bool KoDocumentSectionDelegate::eventFilter( QObject *object, QEvent *event )
{
    QLineEdit *edit = qobject_cast<QLineEdit*>( object );
    if( edit && event->type() == QEvent::KeyPress )
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

QRect KoDocumentSectionDelegate::textRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    static QFont f;
    static int minbearing = 1337 + 666; //can be 0 or negative, 2003 is less likely
    if( minbearing == 2003 || f != option.font )
    {
        f = option.font; //getting your bearings can be expensive, so we cache them
        minbearing = option.fontMetrics.minLeftBearing() + option.fontMetrics.minRightBearing();
    }

    int indent = thumbnailRect( option, index ).right() - option.rect.left() + d->margin;
    if( !index.data( Qt::DecorationRole ).value<QIcon>().isNull() )
        indent += option.decorationSize.width() + d->margin;

    const int width = ( d->mode == DetailedMode ? option.rect.width() : iconsRect( option, index ).left() - option.rect.left() ) - indent - d->margin + minbearing;

    return QRect( indent, 0, width, option.fontMetrics.height() ).translated( option.rect.topLeft() );
}

QRect KoDocumentSectionDelegate::iconsRect( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    Model::PropertyList lp = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    int propscount = 0;
    for( int i = 0, n = lp.count(); i < n; ++i )
        if( lp[i].isMutable )
            propscount++;

    const int iconswidth = propscount * option.decorationSize.width() + (propscount - 1) * d->margin;

    const int x = d->mode == DetailedMode ? thumbnailRect( option, index ).right() - option.rect.left() + d->margin : option.rect.width() - iconswidth;
    const int y = d->mode == DetailedMode ? option.fontMetrics.height() : 0;

    return QRect( x, y, iconswidth, option.decorationSize.height() ).translated( option.rect.topLeft() );
}

QRect KoDocumentSectionDelegate::thumbnailRect( const QStyleOptionViewItem &option, const QModelIndex & ) const
{
    return QRect( 0, 0, option.rect.height(), option.rect.height() ).translated( option.rect.topLeft() );
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

    const QImage i = index.data( Model::ThumbnailRole ).value<QImage>()
                     .scaled( r.height(), r.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    QPoint offset;
    offset.setX( r.width()/2 - i.width()/2 );
    offset.setY( r.height()/2 - i.height()/2 );

    p->drawImage( r.topLeft() + offset, i );
}


// TOOLTIPS


KoDocumentSectionDelegate::ToolTip *KoDocumentSectionDelegate::ToolTip::instance = 0;

void KoDocumentSectionDelegate::ToolTip::showTip( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index )
{
    if( !instance )
        instance = new ToolTip();

    instance->update( widget, pos, option, index );
}

void KoDocumentSectionDelegate::ToolTip::hideTip()
{
    if( !instance )
        return;

    instance->hide();
    delete instance;
}

KoDocumentSectionDelegate::ToolTip::ToolTip()
{
    instance = this;
    setWindowFlags( Qt::FramelessWindowHint  | Qt::Tool
                  | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );
    setPalette( QToolTip::palette() );
    QApplication::instance()->installEventFilter( this );
}

KoDocumentSectionDelegate::ToolTip::~ToolTip()
{
    instance = 0;
}

void KoDocumentSectionDelegate::ToolTip::update( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index )
{
    updateDocument( option, index );
    updatePosition( widget, pos, option );
    show();
}

void KoDocumentSectionDelegate::ToolTip::updateDocument( const QStyleOptionViewItem &option, const QModelIndex &index )
{
    m_document.clear();

    QImage thumb = index.data( Model::LargeThumbnailRole ).value<QImage>();
    m_document.addResource( QTextDocument::ImageResource, QUrl( "data:thumbnail" ), thumb );

    QString name = index.data( Qt::DisplayRole ).toString();
    Model::PropertyList properties = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    QString rows;
    for( int i = 0, n = properties.count(); i < n; ++i )
    {
        const QString row = QString( "<tr><td align=\"right\">%1</td><td align=\"left\">%2</td></tr>" );
        const QString value = properties[i].isMutable
                      ? ( properties[i].state.toBool() ? i18n( "Yes" ) : i18n( "No" ) )
                      : properties[i].state.toString();
        rows.append( row.arg( i18n( "%1:", properties[i].name ) ).arg( value ) );
    }

    rows = QString( "<table>%1</table>" ).arg( rows );

    const QString image = QString( "<table border=\"1\"><tr><td><img src=\"data:thumbnail\"></td></tr></table>" );
    const QString body = QString( "<h3 align=\"center\">%1</h3>" ).arg( name )
                       + QString( "<table><tr><td>%1</td><td>%2</td></tr></table>" ).arg( image ).arg( rows );
    const QString html = QString( "<html><body>%1</body></html>" ).arg( body );

    m_document.setHtml( html );
    m_document.setTextWidth( qMin( m_document.size().width(), 500.0 ) );
}

void KoDocumentSectionDelegate::ToolTip::updatePosition( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option )
{
    const QRect drect = QApplication::desktop()->availableGeometry( widget );
    const QSize size = sizeHint();
    const int width = size.width(), height = size.height();
    const QPoint gpos = widget->mapToGlobal( pos );
    const QRect irect( widget->mapToGlobal( option.rect.topLeft() ), option.rect.size() );

    int y;
    if( irect.bottom() + height < drect.bottom() )
        y = irect.bottom();
    else
        y = qMax( drect.top(), irect.top() - height );

    int x;
    if( gpos.x() + width < drect.right() )
        x = gpos.x();
    else
        x = qMax( drect.left(), gpos.x() - width );

    move( x, y );
}

QSize KoDocumentSectionDelegate::ToolTip::sizeHint() const
{
    return m_document.size().toSize();
}

void KoDocumentSectionDelegate::ToolTip::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    p.initFrom( this );
    m_document.drawContents( &p, rect() );
    p.drawRect( 0, 0, width() - 1, height() - 1 );
}

bool KoDocumentSectionDelegate::ToolTip::eventFilter( QObject *object, QEvent *event )
{
    switch( event->type() )
    {
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Enter:
        case QEvent::Leave:
            hide();
            deleteLater();
        default: break;
    }

    return super::eventFilter( object, event );
}

#include "KoDocumentSectionDelegate.moc"
