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

#include <iostream>

#include <komAutoLoader.h>
#include <kom.h>

#include <qmsgbox.h>

typedef KOMAutoLoader<Factory> MyAutoLoader;

MyApplication::MyApplication( int &argc, char **argv ) : KOMApplication( argc, argv, "makro_recorder")
{
}

void MyApplication::start()
{
}

Factory::Factory( const CORBA::ORB::ObjectTag &_tag ) : KOM::PluginFactory_skel( _tag )
{
}

Factory::Factory( CORBA::Object_ptr _obj ) : KOM::PluginFactory_skel( _obj )
{
}

KOM::Plugin_ptr Factory::create( const KOM::Component_ptr _comp )
{
  CORBA::Object_var obj = _comp->getInterface( "IDL:OpenParts/View:1.0" );
  if ( CORBA::is_nil( obj ) )
    return 0L;
  OpenParts::View_var view = OpenParts::View::_narrow( obj );
  if( CORBA::is_nil( view ) )
    return 0L;
    
  cerr << "CREATING MakroRecorder" << endl;
  
  return KOM::Plugin::_duplicate( new Recorder( view ) );
}

Recorder::Recorder( OpenParts::View_ptr _view )
{
  cerr << "Created MakroRecorder" << endl;
  
  m_status = ST_STOP;
  m_vView = OpenParts::View::_duplicate( _view );
  m_lstTape.setAutoDelete( true );
}

void Recorder::start()
{
  if ( m_status == ST_PLAY )
  {
    cerr << "PLAYING BACK" << endl;
    return;
  }
  
  cerr << "!!!!!!START!!!!!!!" << endl;

  if ( m_status != ST_STOP )
    stop();

  m_lstTape.clear();
  
  KOM::EventTypeSeq seq;
  seq.length(1);
  seq[0] = CORBA::string_dup( "UserEvent/KSpread/*" );
  
  m_vView->installFilter( this, "eventFilter", seq, KOM::Base::FM_READ );

  m_status = ST_START;
}

void Recorder::stop()
{
  cerr << "!!!!!!STOP!!!!!!!" << endl;

  if ( m_status == ST_STOP )
    return;

  m_vView->uninstallFilter( this );

  m_status = ST_STOP;
}

void Recorder::play()
{
  cerr << "!!!!!!PLAY!!!!!!!" << endl;

  if ( m_status != ST_STOP )
    stop();

  m_status = ST_PLAY;

  Entry* e;
  for( e = m_lstTape.first(); e != 0L; e = m_lstTape.next() )
    m_vView->receive( e->m_strType, e->m_any );
  
  m_status = ST_STOP;
}
  
CORBA::Boolean Recorder::eventFilter( KOM::Base_ptr _obj, const char* _type, const CORBA::Any& _value )
{
  cerr << "GOT Event " << _type << endl;

  Entry* e = new Entry;
  e->m_any = _value;
  e->m_strType = _type;
  m_lstTape.append( e );
  
  return false;
}

int main( int argc, char **argv )
{
  MyApplication app( argc, argv );

  MyAutoLoader loader( "IDL:KOM/PluginFactory:1.0", "MakroRecorder" );
  
  app.exec();

  return 0;
}

#include "main.moc"

