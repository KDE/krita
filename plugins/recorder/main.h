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

#ifndef __my_app_h__
#define __my_app_h__

#include <komApplication.h>
#include <komPlugin.h>
#include <kom.h>

#include "openparts.h"
#include "koView.h"
#include "recorder.h"

#include <qlist.h>
#include <qstring.h>

#include <string>
#include <map>

class MyApplication : public KOMApplication
{
  Q_OBJECT
public:
  MyApplication( int &argc, char **argv );
  
  void start();
};

class Factory : virtual public KOM::PluginFactory_skel
{
public:
  Factory( const CORBA::ORB::ObjectTag &_tag );
  Factory( CORBA::Object_ptr _obj );

  KOM::Plugin_ptr create( const KOM::Component_ptr _comp );
};

class Recorder : virtual public KOMPlugin,
		 virtual public MakroRecorder::Recorder_skel
{
public:
  Recorder( OpenParts::View_ptr );

  virtual void start();
  virtual void stop();
  virtual void play();
  
  virtual CORBA::Boolean eventFilter( ::KOM::Base_ptr obj, const char* type, const CORBA::Any& value );

protected:
  enum Status { ST_STOP, ST_START, ST_PLAY };
  
  Status m_status;
  OpenParts::View_var m_vView;

  struct Entry
  {
    CORBA::Any m_any;
    QString m_strType;
    int m_id;
  };

  QList<Entry> m_lstTape;
  int m_id;
  map<string,int> m_mapObjects;
  map<int,KOM::Base_var> m_mapIds;
};

#endif
