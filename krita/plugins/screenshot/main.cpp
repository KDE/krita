#include <kapplication.h>
#include "ksnapshot.h"
#include <kimageio.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kiconloader.h>

static const char description[] =
    I18N_NOOP("KDE Screenshot Utility");

int main(int argc, char **argv)
{
  KAboutData aboutData( "ksnapshot", I18N_NOOP("KSnapshot"),
    KSNAPVERSION, description, KAboutData::License_GPL,
    "(c) 1997-2003, Richard J. Moore,\n(c) 2000, Matthias Ettrich,\n(c) 2002-2003 Aaron J. Seigo");
  aboutData.addAuthor("Richard J. Moore",0, "rich@kde.org");
  aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
  aboutData.addAuthor("Aaron J. Seigo", 0, "aseigo@kde.org");
  aboutData.addCredit( "Nadeem Hasan", "Region Grabbing\nReworked GUI",
      "nhasan@kde.org" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  KImageIO::registerFormats();

  // Create top level window
  KSnapshot *toplevel= new KSnapshot();
  Q_CHECK_PTR(toplevel);
  app.dcopClient()->setDefaultObject( toplevel->objId() );
  toplevel->setCaption( app.makeStdCaption("") );
  toplevel->setIcon(SmallIcon("tool_screenshot"));
  app.setMainWidget(toplevel);
  toplevel->show();
  return app.exec();
}

