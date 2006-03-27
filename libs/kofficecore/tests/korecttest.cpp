#include <KoRect.h>
#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kglobal.h>

bool check(QString txt, bool res, bool expected)
{
  if (res == expected) {
    kDebug() << txt << " : checking '" << res << "' against expected value '" << expected << "'... " << "ok" << endl;
  }
  else {
    kDebug() << txt << " : checking '" << res << "' against expected value '" << expected << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

bool check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"korecttest",false,false);

  KoRect emptyRect;
  check( "KoRect() is null", emptyRect.isNull(), true );
  check( "KoRect() is empty", emptyRect.isEmpty(), true );
  KoRect rect( 1, 15, 250, 156.14 );
  check( "KoRect(...) is not null", rect.isNull(), false );
  check( "KoRect(...) is not empty", rect.isEmpty(), false );
  KoRect unionRect = rect | emptyRect;
  check( "Union is not null", unionRect.isNull(), false );
  check( "Union is not empty", unionRect.isEmpty(), false );
  kDebug() << unionRect << endl;

  printf("\nTest OK !\n");
}

