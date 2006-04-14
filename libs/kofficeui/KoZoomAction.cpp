/*  This file is part of the KDE libraries
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include <KoZoomAction.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <q3valuelist.h>

#include <klocale.h>
#include <q3tl.h>

KoZoomAction::KoZoomAction( const QString& text, const QIcon& pix,
  const KShortcut& cut, KActionCollection* parent, const char* name ):
  KSelectAction( text, pix, cut, parent, name )
{
  init();
}

KoZoomAction::KoZoomAction( const QString& text, const QString& pix,
  const KShortcut& cut, KActionCollection* parent, const char* name ):
  KSelectAction( text, pix, cut, parent, name )
{
  init();

}

void KoZoomAction::setZoom( const QString& text )
{
  bool ok = false;
  QString t = text;
  int zoom = t.remove( '%' ).toInt( &ok );

  // where we'll store sorted new zoom values
  Q3ValueList<int> list;
  if( zoom > 10 ) list.append( zoom );

  // "Captured" non-empty sequence of digits
  QRegExp regexp("(\\d+)");

  const QStringList itemsList( items() );
  for( QStringList::ConstIterator it = itemsList.begin(); it != itemsList.end(); ++it )
  {
    regexp.search( *it );
    const int val=regexp.cap(1).toInt( &ok );

    //zoom : limit inferior=10
    if( ok && val>9 && list.contains( val )==0 )
      list.append( val );
  }

  qHeapSort( list );

  // update items with new sorted zoom values
  QStringList values;
  for (Q3ValueList<int>::Iterator it = list.begin(); it != list.end(); ++it )
    values.append( i18n("%1%", *it) );
  setItems( values );

  QString zoomStr = i18n("%1%",  zoom );
  setCurrentItem( values.findIndex( zoomStr ) );
}

void KoZoomAction::setZoom( int zoom )
{
  setZoom( QString::number( zoom ) );
}

void KoZoomAction::activated( const QString& text )
{
  setZoom( text );
  emit zoomChanged( text );
}

void KoZoomAction::init()
{
  setEditable( true );

  QStringList values;
  values << i18n("%1%", QString("33"));
  values << i18n("%1%", QString("50"));
  values << i18n("%1%", QString("75"));
  values << i18n("%1%", QString("100"));
  values << i18n("%1%", QString("125"));
  values << i18n("%1%", QString("150"));
  values << i18n("%1%", QString("200"));
  values << i18n("%1%", QString("250"));
  values << i18n("%1%", QString("350"));
  values << i18n("%1%", QString("400"));
  values << i18n("%1%", QString("450"));
  values << i18n("%1%", QString("500"));
  setItems( values );

  setCurrentItem( values.findIndex( i18n("%1%",  100 ) ) );

  connect( this, SIGNAL( activated( const QString& ) ),
    SLOT( activated( const QString& ) ) );
}

#include "KoZoomAction.moc"
