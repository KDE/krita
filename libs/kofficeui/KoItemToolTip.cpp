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

#include <QApplication>
#include <QBasicTimer>
#include <QDesktopWidget>
#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QPersistentModelIndex>
#include <QStyleOptionViewItem>
#include <QTextDocument>
#include <QTimerEvent>
#include <QToolTip>
#include "KoItemToolTip.h"

class KoItemToolTip::Private
{
    public:
        QTextDocument *document;
        QPersistentModelIndex index;
        QPoint pos;
        QBasicTimer timer;

        Private(): document( 0 ) { }
};

KoItemToolTip::KoItemToolTip()
    : d( new Private )
{
    d->document = new QTextDocument( this );
    setWindowFlags( Qt::FramelessWindowHint  | Qt::Tool
                  | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );
    setPalette( QToolTip::palette() );
    QApplication::instance()->installEventFilter( this );
}

KoItemToolTip::~KoItemToolTip()
{
    delete d;
}

void KoItemToolTip::showTip( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index )
{
    QTextDocument *doc = createDocument( index );

    QPoint p = ( isVisible() && index == d->index ) ? d->pos : pos;

    if( !isVisible() || index != d->index || doc->toHtml() != d->document->toHtml() )
    {
        d->pos = p;
        d->index = index;
        delete d->document;
        d->document = doc;
        updatePosition( widget, p, option );
        if( !isVisible() )
            show();
        else
            update();
        d->timer.start( 10000, this );
    }
    else
        delete doc;
}

void KoItemToolTip::updatePosition( QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option )
{
    const QRect drect = QApplication::desktop()->availableGeometry( widget );
    const QSize size = sizeHint();
    const int width = size.width(), height = size.height();
    const QPoint gpos = widget->mapToGlobal( pos );
    const QRect irect( widget->mapToGlobal( option.rect.topLeft() ), option.rect.size() );

    int y = gpos.y() + 20;
    if( y + height > drect.bottom() )
        y = qMax( drect.top(), irect.top() - height );

    int x;
    if( gpos.x() + width < drect.right() )
        x = gpos.x();
    else
        x = qMax( drect.left(), gpos.x() - width );

    move( x, y );

    resize( sizeHint() );
 }

QSize KoItemToolTip::sizeHint() const
{
    return d->document->size().toSize();
}

void KoItemToolTip::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    p.initFrom( this );
    d->document->drawContents( &p, rect() );
    p.drawRect( 0, 0, width() - 1, height() - 1 );
}

void KoItemToolTip::timerEvent( QTimerEvent *e )
{
    if( e->timerId() == d->timer.timerId() )
    {
        hide();
    }
}

bool KoItemToolTip::eventFilter( QObject *object, QEvent *event )
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
        default: break;
    }

    return super::eventFilter( object, event );
}

#include "KoItemToolTip.moc"
