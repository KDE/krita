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
  if( KoZoomMode::isConstant( text ) )
  {
    regenerateItems( text );
  }

  setCurrentAction( text );
}

void KoZoomAction::setZoom( int zoom )
{
  setZoom( QString::number( zoom ) );
}

void KoZoomAction::triggered( const QString& text )
{
  setZoom( text );
  emit zoomChanged( text );
}

void KoZoomAction::init()
{
  setEditable( true );

  QStringList values;

  if( m_zoomModes & KoZoomMode::ZOOM_WIDTH )
  {
    values << KoZoomMode::toString(KoZoomMode::ZOOM_WIDTH);
  }
  if( m_zoomModes & KoZoomMode::ZOOM_PAGE )
  {
    values << KoZoomMode::toString(KoZoomMode::ZOOM_PAGE);
  }

  values << generateZoomLevels();

  setItems( values );

  setCurrentAction( i18n("%1%",  100 ) );

  connect( this, SIGNAL( triggered( const QString& ) ),
    SLOT( triggered( const QString& ) ) );
}

void KoZoomAction::setZoomModes( KoZoomMode::Modes zoomModes )
{
  m_zoomModes = zoomModes;
  regenerateItems( currentText() );
}

QStringList KoZoomAction::generateZoomLevels()
{
  QStringList values;
  values << i18n("%1%", 33);
  values << i18n("%1%", 50);
  values << i18n("%1%", 75);
  values << i18n("%1%", 100);
  values << i18n("%1%", 125);
  values << i18n("%1%", 150);
  values << i18n("%1%", 200);
  values << i18n("%1%", 250);
  values << i18n("%1%", 350);
  values << i18n("%1%", 400);
  values << i18n("%1%", 450);
  values << i18n("%1%", 500);

  return values;
}

void KoZoomAction::regenerateItems(const QString& zoomString)
{
  QString t = zoomString;
  bool ok = false;
  int zoom = t.remove( '%' ).toInt( &ok );

  // where we'll store sorted new zoom values
  QStringList zoomLevels = generateZoomLevels();

  if( ok && zoom > 10 && !zoomLevels.contains( zoomString ) )
    zoomLevels.append( zoomString );

  zoomLevels.sort();

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

  values << zoomLevels;

  setItems( values );
}

#include "KoZoomAction.moc"
