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
  
  m_id = 0;
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
  m_mapObjects.clear();
  m_mapIds.clear();
  m_id = 0;
  
  KOM::EventTypeSeq seq;
  seq.length(2);
  seq[0] = CORBA::string_dup( "UserEvent/*" );
  seq[1] = CORBA::string_dup( KOffice::View::eventNewPart );

  CORBA::String_var str = komapp_orb->object_to_string( m_vView );
  m_mapObjects[ str.in() ] = m_id;
  m_mapIds[ m_id ] = KOM::Base::_duplicate( m_vView );

  m_vView->installFilter( this, "eventFilter", seq, KOM::Base::FM_READ );

  m_status = ST_START;
}

void Recorder::stop()
{
  cerr << "!!!!!!STOP!!!!!!!" << endl;

  if ( m_status == ST_STOP )
    return;

  // Uninstall all event filters
  map<int,KOM::Base_var>::iterator it = m_mapIds.begin();
  for( ; it != m_mapIds.end(); ++it )
  {    
    it->second->uninstallFilter( this );
  }
  
  m_mapObjects.clear();
  m_mapIds.clear();
  m_id = 0;

  m_status = ST_STOP;
}

void Recorder::play()
{
  cerr << "!!!!!!PLAY!!!!!!!" << endl;

  if ( m_status != ST_STOP )
    stop();

  m_status = ST_PLAY;
  
  m_mapObjects.clear();
  m_mapIds.clear();
  m_id = 0;
  m_mapIds[ m_id ] = KOM::Base::_duplicate( m_vView );

  KOM::EventTypeSeq seq;
  seq.length(1);
  seq[0] = CORBA::string_dup( KOffice::View::eventNewPart );
  
  m_vView->installFilter( this, "eventFilter", seq, KOM::Base::FM_READ );
  
  cerr << "RECORDS=" << m_lstTape.count() << endl;

  Entry* e;
  for( e = m_lstTape.first(); e != 0L; e = m_lstTape.next() )
  {  
    map<int,KOM::Base_var>::iterator it = m_mapIds.find( e->m_id );
    if ( it == m_mapIds.end() )
    {
      cerr << "Did not find receiver for id " << e->m_id << endl;
      return;
    }
    
    cerr << "SENDING=" << e->m_strType << endl;
    
    it->second->receive( e->m_strType, e->m_any );
  }
  
  stop();
}
  
CORBA::Boolean Recorder::eventFilter( KOM::Base_ptr _obj, const char* _type,
				      const CORBA::Any& _value )
{
  cerr << "GOT Event " << _type << endl;

  CORBA::String_var str = komapp_orb->object_to_string( _obj );

  if ( strcmp( _type, KOffice::View::eventNewPart ) == 0L )
  {
    KOffice::View::EventNewPart event;
    if ( _value >>= event )
    {  
      if ( m_status == ST_START )
      {
	CORBA::String_var str2 = komapp_orb->object_to_string( event.view );
	m_mapObjects[ str2.in() ] = ++m_id;
	m_mapIds[ m_id ] = KOM::Base::_duplicate( event.view );
	
	KOM::EventTypeSeq seq;
	seq.length(2);
	seq[0] = CORBA::string_dup( "UserEvent/*" );
	seq[1] = CORBA::string_dup( KOffice::View::eventNewPart );

	event.view->installFilter( this, "eventFilter", seq, KOM::Base::FM_READ );
      }
      else if ( m_status == ST_PLAY )
      {
	m_mapIds[ ++m_id ] = KOM::Base::_duplicate( event.view );

	KOM::EventTypeSeq seq;
	seq.length(1);
	seq[0] = CORBA::string_dup( KOffice::View::eventNewPart );

	event.view->installFilter( this, "eventFilter", seq, KOM::Base::FM_READ );
      }
    }
    return false;
  }

  map<string,int>::iterator it = m_mapObjects.find( str.in() );
  if ( it == m_mapObjects.end() )
  {
    cerr << "ERROR: Unknown new obejct" << endl;
    return false;
  }
  
  Entry* e = new Entry;
  e->m_any = _value;
  e->m_strType = _type;
  e->m_id = it->second;
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
