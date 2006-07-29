/*  This file is part of the KDE libraries
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>

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

#include <KoZoomAction.h>

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QList>

#include <klocale.h>
#include <kicon.h>

#include <KoZoomMode.h>

KoZoomAction::KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, const QIcon& pix,
  const KShortcut& cut, KActionCollection* parent, const char* name ):
  KSelectAction( KIcon(pix), text, parent, name ), m_zoomModes( zoomModes )
{
  setShortcut(cut);

  init();
}

KoZoomAction::KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, const QString& pix,
  const KShortcut& cut, KActionCollection* parent, const char* name ):
  KSelectAction( KIcon(pix), text, parent, name ), m_zoomModes( zoomModes )
{
  setShortcut(cut);

  init();
}

void KoZoomAction::setZoom( const QString& text )
{
  QString zoomString = text;
  zoomString = zoomString.remove('&');
  if( KoZoomMode::isConstant( zoomString ) )
  {
    regenerateItems( zoomString );
  }

  setCurrentAction( zoomString );
}

void KoZoomAction::setZoom( int zoom )
{
  setZoom( i18n( "%1%", zoom ) );
}

void KoZoomAction::triggered( const QString& text )
{
  QString zoomString = text;
  zoomString = zoomString.remove( '&' );

  KoZoomMode::Mode mode = KoZoomMode::toMode( zoomString );
  int zoom = 0;

  if( mode == KoZoomMode::ZOOM_CONSTANT ) {
    bool ok;
    QRegExp regexp( ".*(\\d+).*" ); // "Captured" non-empty sequence of digits
    int pos = regexp.indexIn( zoomString );

    if( pos > -1 ) {
      zoom = regexp.cap( 1 ).toInt( &ok );

      if( !ok ) {
        zoom = 0;
      }
    }
  }

  emit zoomChanged( mode, zoom );
}

void KoZoomAction::init()
{
  setEditable( true );
  setMaxComboViewCount( 15 );

  regenerateItems(0);

  setCurrentAction( i18n( "%1%",  100 ) );

  connect( this, SIGNAL( triggered( const QString& ) ),
    SLOT( triggered( const QString& ) ) );
}

void KoZoomAction::setZoomModes( KoZoomMode::Modes zoomModes )
{
  m_zoomModes = zoomModes;
  regenerateItems( currentText() );
}

void KoZoomAction::regenerateItems(const QString& zoomString)
{
  QString t = zoomString;
  bool ok = false;
  int zoom = t.remove( '%' ).toInt( &ok );

  // where we'll store sorted new zoom values
  QList<int> zoomLevels;
  zoomLevels << 33;
  zoomLevels << 50;
  zoomLevels << 75;
  zoomLevels << 100;
  zoomLevels << 125;
  zoomLevels << 150;
  zoomLevels << 200;
  zoomLevels << 250;
  zoomLevels << 350;
  zoomLevels << 400;
  zoomLevels << 450;
  zoomLevels << 500;

  if( ok && zoom > 10 && !zoomLevels.contains( zoom ) )
    zoomLevels << zoom;

  qSort(zoomLevels.begin(), zoomLevels.end());

    // update items with new sorted zoom values
  QStringList values;
  if(m_zoomModes & KoZoomMode::ZOOM_WIDTH)
  {
    values << KoZoomMode::toString(KoZoomMode::ZOOM_WIDTH);
  }
  if(m_zoomModes & KoZoomMode::ZOOM_PAGE)
  {
    values << KoZoomMode::toString(KoZoomMode::ZOOM_PAGE);
  }

  foreach(int value, zoomLevels) {
    values << i18n("%1%", value);
  }

  setItems( values );
}

#include "KoZoomAction.moc"
