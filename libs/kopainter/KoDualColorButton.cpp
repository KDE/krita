/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include <QtGui/QBitmap>
#include <QtGui/QBrush>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QPainter>

#include <kglobalsettings.h>

#include "kcolormimedata.h"

#include "KoDualColorButton.h"
#include "KoUniColorDialog.h"
#include "KoColor.h"

#include "dcolorarrow.xbm"
#include "dcolorreset.xpm"

class KoDualColorButton::Private
{
  public:
    Private(const KoColor &fgColor, const KoColor &bgColor)
        : dragFlag( false )
        , miniCtlFlag( false )
        , foregroundColor(fgColor)
        , backgroundColor(bgColor)
    {
        arrowBitmap = QBitmap::fromData( QSize(dcolorarrow_width, dcolorarrow_height),
                                       (const unsigned char *)dcolorarrow_bits, QImage::Format_MonoLSB );
        arrowBitmap.setMask( arrowBitmap );
        resetPixmap = QPixmap( (const char **)dcolorreset_xpm );

        popDialog = true;
    }

    QWidget* dialogParent;

    QBitmap arrowBitmap;
    QPixmap resetPixmap;
    bool dragFlag, miniCtlFlag;
    KoColor foregroundColor;
    KoColor backgroundColor;
    QPoint dragPosition;
    Selection tmpSelection;
    bool popDialog;
};

KoDualColorButton::KoDualColorButton(const KoColor &foregroundColor, const KoColor &backgroundColor, QWidget *parent, QWidget* dialogParent )
  : QWidget( parent ),
    d( new Private(foregroundColor, backgroundColor) )
{
    d->dialogParent = dialogParent;

    if ( sizeHint().isValid() )
        setMinimumSize( sizeHint() );

    setAcceptDrops( true );
}

KoDualColorButton::~KoDualColorButton()
{
  delete d;
}

KoColor KoDualColorButton::foregroundColor() const
{
  return d->foregroundColor;
}

KoColor KoDualColorButton::backgroundColor() const
{
  return d->backgroundColor;
}

bool KoDualColorButton::popDialog() const
{
  return d->popDialog;
}

QSize KoDualColorButton::sizeHint() const
{
  return QSize( 34, 34 );
}

void KoDualColorButton::setForegroundColor( const KoColor &color )
{
  d->foregroundColor = color;
  repaint();
}

void KoDualColorButton::setBackgroundColor( const KoColor &color )
{
  d->backgroundColor = color;
  repaint();
}

void KoDualColorButton::setPopDialog( bool popDialog )
{
  d->popDialog = popDialog;
}

void KoDualColorButton::metrics( QRect &foregroundRect, QRect &backgroundRect )
{
  foregroundRect = QRect( 0, 0, width() - 14, height() - 14 );
  backgroundRect = QRect( 14, 14, width() - 14, height() - 14 );
}

void KoDualColorButton::paintEvent(QPaintEvent *)
{
  QRect foregroundRect;
  QRect backgroundRect;

  QPainter painter( this );

  metrics( foregroundRect, backgroundRect );

  QBrush defBrush = palette().brush( QPalette::Button );
  QBrush foregroundBrush( d->foregroundColor.toQColor(), Qt::SolidPattern );
  QBrush backgroundBrush( d->backgroundColor.toQColor(), Qt::SolidPattern );

  qDrawShadeRect( &painter, backgroundRect, palette(), false, 2, 0,
                  isEnabled() ? &backgroundBrush : &defBrush );

  qDrawShadeRect( &painter, foregroundRect, palette(), false, 2, 0,
                  isEnabled() ? &foregroundBrush : &defBrush );

  painter.setPen( palette().color( QPalette::Shadow ) );

  painter.drawPixmap( foregroundRect.right() + 2, 0, d->arrowBitmap );
  painter.drawPixmap( 0, foregroundRect.bottom() + 2, d->resetPixmap );
}

void KoDualColorButton::dragEnterEvent( QDragEnterEvent *event )
{
  event->setAccepted( isEnabled() && KColorMimeData::canDecode( event->mimeData() ) );
}

void KoDualColorButton::dropEvent( QDropEvent *event )
{
/*  QColor color = KColorMimeData::fromMimeData( event->mimeData() );

  if ( color.isValid() ) {
    if ( d->selection == Foreground ) {
      d->foregroundColor = color;
      emit foregroundColorChanged( color );
    } else {
      d->backgroundColor = color;
      emit backgroundColorChanged( color );
    }

    repaint();
  }
*/
}

void KoDualColorButton::mousePressEvent( QMouseEvent *event )
{
  QRect foregroundRect;
  QRect backgroundRect;

  metrics( foregroundRect, backgroundRect );

  d->dragPosition = event->pos();

  d->dragFlag = false;

  if ( foregroundRect.contains( d->dragPosition ) ) {
    d->tmpSelection = Foreground;
    d->miniCtlFlag = false;
  } else if( backgroundRect.contains( d->dragPosition ) ) {
    d->tmpSelection = Background;
    d->miniCtlFlag = false;
  } else if ( event->pos().x() > foregroundRect.width() ) {
    // We handle the swap and reset controls as soon as the mouse is
    // is pressed and ignore further events on this click (mosfet).

    KoColor tmp = d->foregroundColor;
    d->foregroundColor = d->backgroundColor;
    d->backgroundColor = tmp;

    emit foregroundColorChanged( d->foregroundColor );
    emit backgroundColorChanged( d->backgroundColor );

    d->miniCtlFlag = true;
  }
  repaint();
}


void KoDualColorButton::mouseMoveEvent( QMouseEvent *event )
{
  if ( !d->miniCtlFlag ) {
    int delay = KGlobalSettings::dndEventDelay();

    if ( event->x() >= d->dragPosition.x() + delay || event->x() <= d->dragPosition.x() - delay ||
         event->y() >= d->dragPosition.y() + delay || event->y() <= d->dragPosition.y() - delay ) {
      KColorMimeData::createDrag( d->tmpSelection == Foreground ?
                                  d->foregroundColor.toQColor() : d->backgroundColor.toQColor(),
                                  this )->start();
      d->dragFlag = true;
    }
  }
}

void KoDualColorButton::mouseReleaseEvent( QMouseEvent *event )
{
    d->dragFlag = false;

    if ( d->miniCtlFlag )
        return;

    d->miniCtlFlag = false;

    QRect foregroundRect;
    QRect backgroundRect;
    metrics( foregroundRect, backgroundRect );

    if ( foregroundRect.contains( event->pos() )) {
        if(d->tmpSelection == Foreground ) {
            if( d->popDialog) {
                KoUniColorDialog *dialog = new KoUniColorDialog(d->foregroundColor, d->dialogParent);
                if(dialog->exec() != KPageDialog::Accepted) {
                 //   d->foregroundColor = newColor;
                    emit foregroundColorChanged( d->foregroundColor );
                }
            }
            else
                emit pleasePopDialog( d->foregroundColor);
        }
        else {
            d->foregroundColor = d->backgroundColor;
            emit foregroundColorChanged( d->foregroundColor );
        }
    } else if ( backgroundRect.contains( event->pos() )) {
        if(d->tmpSelection == Background ) {
            if( d->popDialog) {
                KoUniColorDialog *dialog = new KoUniColorDialog(d->backgroundColor, d->dialogParent);
                if(dialog->exec() != KPageDialog::Accepted) {
                    //d->backgroundColor = ;
                    emit backgroundColorChanged( d->backgroundColor );
                }
            }
            else
                emit pleasePopDialog( d->backgroundColor);
        } else {
            d->backgroundColor = d->foregroundColor;
            emit backgroundColorChanged( d->backgroundColor );
        }
    }

    repaint();
}

#include "KoDualColorButton.moc"
