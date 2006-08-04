/*
 * This file was copied from ksnapshot
 *
 * Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "regiongrabber.h"

#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QTimer>
#include <QToolTip>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QLabel>
#include <QFrame>
#include <QKeyEvent>

#include <kglobalsettings.h>
#include <QX11Info>

SizeTip::SizeTip( QWidget *parent, const char *name )
    : QLabel( parent, name, Qt::WStyle_Customize | Qt::WX11BypassWM |
      Qt::WStyle_StaysOnTop | Qt::WStyle_NoBorder | Qt::WStyle_Tool )
{
  setMargin( 2 );
  setIndent( 0 );
  setFrameStyle( QFrame::Plain | QFrame::Box );

  setPalette( QToolTip::palette() );
}

void SizeTip::setTip( const QRect &rect )
{
  QString tip = QString( "%1x%2" ).arg( rect.width() )
      .arg( rect.height() );

  setText( tip );
  adjustSize();

  positionTip( rect );
}

void SizeTip::positionTip( const QRect &rect )
{
  QRect tipRect = geometry();
  tipRect.moveTopLeft( QPoint( 0, 0 ) );

  if ( rect.intersects( tipRect ) )
  {
    QRect deskR = KGlobalSettings::desktopGeometry( QPoint( 0, 0 ) );

    tipRect.moveCenter( QPoint( deskR.width()/2, deskR.height()/2 ) );
    if ( !rect.contains( tipRect, true ) && rect.intersects( tipRect ) )
      tipRect.moveBottomRight( geometry().bottomRight() );
  }

  move( tipRect.topLeft() );
}

RegionGrabber::RegionGrabber()
  : QWidget( 0, 0 ),
    mouseDown( false ), sizeTip( 0L )
{
  sizeTip = new SizeTip( ( QWidget * )0L );

  tipTimer = new QTimer( this );
  Q_CHECK_PTR(tipTimer);
  connect( tipTimer, SIGNAL( timeout() ), SLOT( updateSizeTip() ) );

  QTimer::singleShot( 200, this, SLOT( initGrabber() ) );
}

RegionGrabber::~RegionGrabber()
{
  delete sizeTip;
}

void RegionGrabber::initGrabber()
{
  pixmap = QPixmap::grabWindow( QX11Info::appRootWindow() );
  setPaletteBackgroundPixmap( pixmap );

  showFullScreen();

  grabMouse( Qt::CrossCursor );
}

void RegionGrabber::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    mouseDown = true;
    grabRect = QRect( e->pos(), e->pos() );
  }
}

void RegionGrabber::mouseMoveEvent( QMouseEvent *e )
{
  if ( mouseDown )
  {
    sizeTip->hide();
    tipTimer->setSingleShot(true);
    tipTimer->start( 250 );

    drawRubber();
    grabRect.setBottomRight( e->pos() );
    drawRubber();
  }
}

void RegionGrabber::mouseReleaseEvent( QMouseEvent *e )
{
  mouseDown = false;
  drawRubber();
  sizeTip->hide();

  grabRect.setBottomRight( e->pos() );
  grabRect = grabRect.normalize();

  QPixmap region = QPixmap::grabWindow( winId(), grabRect.x(), grabRect.y(),
      grabRect.width(), grabRect.height() );

  releaseMouse();

  emit regionGrabbed( region );
}

void RegionGrabber::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    releaseMouse();
    emit regionGrabbed( QPixmap() );
  }
  else
    e->ignore();
}

void RegionGrabber::updateSizeTip()
{
  QRect rect = grabRect.normalize();

  sizeTip->setTip( rect );
  sizeTip->show();
}

void RegionGrabber::drawRubber()
{
  QPainter p;
  p.begin( this );
  //p.setRasterOp( NotROP );
  p.setPen( QPen( Qt::color0, 1 ) );
  p.setBrush( Qt::NoBrush );

#warning kde4 port
//   style()->drawPrimitive( QStyle::PE_FocusRect, &p, grabRect, colorGroup(),
//       QStyle::State_None, QStyleOption( colorGroup().base() ) );

  p.end();
}

#include "regiongrabber.moc"
