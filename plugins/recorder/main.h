#ifndef __my_app_h__
#define __my_app_h__

#include <komApplication.h>
#include <komBase.h>
#include <kom.h>

#include "openparts.h"
#include "koView.h"
#include "recorder.h"

#include <qlist.h>
#include <qstring.h>

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

class Recorder : virtual public KOMComponent,
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
  };

  QList<Entry> m_lstTape;
};

#endif
