/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "main.h"
#include "kcalc.h"
#include "kspread.h"

#include <iostream>
#include <string.h>

#include <komAutoLoader.h>
#include <komShutdownManager.h>
#include <kom.h>

#include <qmsgbox.h>
#include <qstring.h>
#include <kglobal.h>
#include <kstddirs.h>

typedef KOMAutoLoader<Factory> MyAutoLoader;

/***************************************************
 *
 * Utilities
 *
 ***************************************************/

QString util_columnLabel( int column )
{
  char buffer[ 100 ];

  if ( column <= 26 )
    sprintf( buffer, "%c", 'A' + column - 1 );
  else if ( column <= 26 * 26 )
    sprintf( buffer, "%c%c",'A'+((column-1)/26)-1,'A'+((column-1)%26));
  else
    strcpy( buffer,"@@@");

  return QString( buffer );
}

QString util_cellName( int _col, int _row )
{
  QString label( util_columnLabel( _col ) );

  char buffer[ 20 ];
  sprintf( buffer, "%s%d", label.data(), _row );

  return QString( buffer );
}

QString util_rangeName( KSpread::Range _area )
{
  QString result( _area.table.in() );
  result += "!";
  result += util_cellName( _area.left, _area.top );
  result += ":";
  result += util_cellName( _area.right, _area.bottom );

  return result;
}

/***************************************************
 *
 * MyApplication
 *
 ***************************************************/

MyApplication::MyApplication( int &argc, char **argv ) :
  KOMApplication( argc, argv, "kocalc")
{
  KGlobal::dirs()->addResourceType("toolbar", KStandardDirs::kde_default("data") + "kcalc/pics/");
}

void MyApplication::start()
{
}

/***************************************************
 *
 * Factory
 *
 ***************************************************/

Factory::Factory( const CORBA::ORB::ObjectTag &_tag ) : KOM::PluginFactory_skel( _tag )
{
  cerr << "FACTORY 1" << endl;
}

Factory::Factory( CORBA::Object_ptr _obj ) : KOM::PluginFactory_skel( _obj )
{
  cerr << "FACTORY 2" << endl;
}

KOM::Plugin_ptr Factory::create( KOM::Component_ptr _comp )
{
  cerr << "Create started" << endl;

  CORBA::Object_var obj = _comp->getInterface( "IDL:OpenParts/View:1.0" );

  cerr << "1" << endl;

  if ( CORBA::is_nil( obj ) )
    return 0L;

  cerr << "2" << endl;

  OpenParts::View_var view = OpenParts::View::_narrow( obj );

  cerr << "3" << endl;

  if( CORBA::is_nil( view ) )
    return 0L;

  cerr << "CREATING Calculator" << endl;

  Calculator *calc = new Calculator( view );
  
  KOMShutdownManager::self()->watchObject( calc );

  return KOM::Plugin::_duplicate( calc );
}

/***************************************************
 *
 * Calculator
 *
 ***************************************************/

Calculator::Calculator( OpenParts::View_ptr _view ) : KOMPlugin( _view )
{
  cerr << "Created Calculator" << endl;

  m_vView = OpenParts::View::_duplicate( _view );
}

QtCalculator* g_calc = 0L;

void Calculator::open()
{
  cerr << "!!!!!! OPEN !!!!!!!" << endl;

  if ( !g_calc )
  {
    g_calc = new QtCalculator( this );
    g_calc->setFixedSize( 9 + 100 + 9 + 233 + 9, 239);
  }

  g_calc->show();

  KOM::EventTypeSeq seq;
  seq.length(1);
  seq[0] = CORBA::string_dup( KSpread::View::eventSelectionChanged );

  m_vView->installFilter( this, "eventFilter", seq, KOM::Base::FM_READ );
}

CORBA::Boolean Calculator::eventFilter( KOM::Base_ptr _obj, const char* _type,
					const CORBA::Any& _value )
{
  if ( strcmp( _type, KSpread::View::eventSelectionChanged ) == 0L )
  {
    KSpread::View::EventSelectionChanged event;
    if ( !( _value >>= event ) )
      return false;

    if ( event.range.left == 0 )
      return false;

    if ( event.range.left == event.range.right &&
	 event.range.top == event.range.bottom )
    {
      KSpread::View_var view = KSpread::View::_narrow( _obj );
      if ( CORBA::is_nil( view ) )
	return false;
      KSpread::Book_var book = view->book();
      if ( CORBA::is_nil( book ) )
	return false;
      KSpread::Table_var table = book->table( event.range.table );
      if ( CORBA::is_nil( table ) )
	return false;

      KSpread::Cell c;
      c.table = CORBA::string_dup( event.range.table );
      c.x = event.range.left;
      c.y = event.range.top;
      CORBA::Double d;
      try
      {
	d = table->value( c );
      }
      catch (...)
      {
	return false;
      }

      g_calc->setValue( d );

      return false;
    }

    QString str = util_rangeName( event.range );

    QRect r;
    r.setCoords( event.range.left, event.range.top, event.range.right, event.range.bottom );
    g_calc->setData( r, event.range.table.in() );
	
    g_calc->setLabel( str );
  }

  return false;
}

void QtCalculator::useData()
{
  KSpread::View_var view = KSpread::View::_narrow( corba->view() );
  if ( CORBA::is_nil( view ) )
    return;
  KSpread::Book_var book = view->book();
  if ( CORBA::is_nil( book ) )
    return;
  KSpread::Table_var table = book->table( table_name );
  if ( CORBA::is_nil( table ) )
    return;

  int len = ( table_range.right() - table_range.left() + 1 ) *
            ( table_range.bottom() - table_range.top() + 1 );
  double *v = new double[ len ];
  int n = 0;
  for( int x = table_range.left(); x <= table_range.right(); x++ )
    for( int y = table_range.top(); y <= table_range.bottom(); y++ )
    {
      bool ok = true;
      KSpread::Cell c;
      c.table = CORBA::string_dup( table_name.data() );
      c.x = x;
      c.y = y;
      CORBA::Double d;
      try
      {
	d = table->value( c );
      }
      catch (...)
      {
	ok = false;
      }
      if ( ok )
	v[n++] = d;
    }

  stats.clearAll();
  for( int i = 0; i < n; i++ )
    stats.enterData( v[i] );

  delete []v;

  table_name = QString::null;
}

int main( int argc, char **argv )
{
  MyApplication app( argc, argv );

  // MyAutoLoader loader( "IDL:KoCalc/Factory:1.0", "KoCalc" );
  MyAutoLoader loader( "IDL:KOM/PluginFactory:1.0", "KoCalc" );

  app.exec();

  return 0;
}

#include "main.moc"

