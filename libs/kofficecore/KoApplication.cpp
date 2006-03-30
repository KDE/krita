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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoApplication.h"
#include <config.h>
#include <qfile.h>
#include <qregexp.h>
#include <dcopclient.h>
#include <KoApplicationIface.h>
#include <KoQueryTrader.h>
#include <KoDocument.h>
#include <KoMainWindow.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <stdlib.h>

void qt_generate_epsf( bool b );

static const KCmdLineOptions options[]=
{
	{"print", I18N_NOOP("Only print and exit"),0},
	{"template", I18N_NOOP("Open a new document with a template"), 0},
	{"dpi <dpiX,dpiY>", I18N_NOOP("Override display DPI"), 0},
	KCmdLineLastOption
};

bool KoApplication::m_starting = true;

class KoApplicationPrivate
{
public:
    KoApplicationPrivate()  {
        m_appIface = 0L;
    }
    KoApplicationIface *m_appIface;  // to avoid a leak
};

KoApplication::KoApplication()
        : KApplication( initHack() )
{
    d = new KoApplicationPrivate;

    // Initialize all KOffice directories etc.
    KoGlobal::initialize();

    // Prepare a DCOP interface
    d->m_appIface = new KoApplicationIface;
    dcopClient()->setDefaultObject( d->m_appIface->objId() );

    m_starting = true;
}

// This gets called before entering KApplication::KApplication
bool KoApplication::initHack()
{
    KCmdLineArgs::addCmdLineOptions( options, I18N_NOOP("KOffice"), "koffice", "kde" );
    return true;
}

// Small helper for start() so that we don't forget to reset m_starting before a return
class KoApplication::ResetStarting
{
public:
    ~ResetStarting()  {
        KoApplication::m_starting = false;
    }
};

bool KoApplication::start()
{
    ResetStarting resetStarting; // reset m_starting to false when we're done
    Q_UNUSED( resetStarting );

    // Find the *.desktop file corresponding to the kapp instance name
    KoDocumentEntry entry = KoDocumentEntry( KoDocument::readNativeService() );
    if ( entry.isEmpty() )
    {
        kError( 30003 ) << instanceName() << "part.desktop not found." << endl;
        kError( 30003 ) << "Run 'kde-config --path services' to see which directories were searched, assuming kde startup had the same environment as your current shell." << endl;
        kError( 30003 ) << "Check your installation (did you install KOffice in a different prefix than KDE, without adding the prefix to /etc/kderc ?)" << endl;
        return false;
    }

    // Get the command line arguments which we have to parse
    KCmdLineArgs *args= KCmdLineArgs::parsedArgs();
    int argsCount = args->count();

    KCmdLineArgs *koargs = KCmdLineArgs::parsedArgs("koffice");
    QString dpiValues = QString::fromLatin1( koargs->getOption( "dpi" ) );
    if ( !dpiValues.isEmpty() ) {
        int sep = dpiValues.find( QRegExp( "[x, ]" ) );
        int dpiX;
        int dpiY = 0;
        bool ok = true;
        if ( sep != -1 ) {
            dpiY = dpiValues.mid( sep+1 ).toInt( &ok );
            dpiValues.truncate( sep );
        }
        if ( ok ) {
            dpiX = dpiValues.toInt( &ok );
            if ( ok ) {
                if ( !dpiY ) dpiY = dpiX;
                KoGlobal::setDPI( dpiX, dpiY );
            }
        }
    }

    // No argument -> create an empty document
    if ( !argsCount ) {
        KoDocument* doc = entry.createDoc( 0, "Document" );
        if ( !doc )
            return false;
        KoMainWindow *shell = new KoMainWindow( doc->instance() );
        shell->show();
        QObject::connect(doc, SIGNAL(sigProgress(int)), shell, SLOT(slotProgress(int)));
        // for initDoc to fill in the recent docs list
        // and for KoDocument::slotStarted
        doc->addShell( shell );

        if ( doc->checkAutoSaveFile() ) {
          shell->setRootDocument( doc );
        } else {
          doc->showStartUpWidget( shell );
        }

        // FIXME This needs to be moved someplace else
	QObject::disconnect(doc, SIGNAL(sigProgress(int)), shell, SLOT(slotProgress(int)));
    } else {
        bool print = koargs->isSet("print");
	bool doTemplate = koargs->isSet("template");
        koargs->clear();

        // Loop through arguments

        short int n=0; // number of documents open
        short int nPrinted = 0;
        for(int i=0; i < argsCount; i++ )
        {
            // For now create an empty document
            KoDocument* doc = entry.createDoc( 0 );
            if ( doc )
            {
                // show a shell asap
                KoMainWindow *shell = new KoMainWindow( doc->instance() );
                if (!print)
                    shell->show();
		// are we just trying to open a template?
		if ( doTemplate ) {
		  QStringList paths;
		  if ( args->url(i).isLocalFile() && QFile::exists(args->url(i).path()) )
		  {
		    paths << QString(args->url(i).path());
		    kDebug(30003) << "using full path..." << endl;
		  } else {
		     QString desktopName(args->arg(i));
		     QString appName = KGlobal::instance()->instanceName();

		     paths = KGlobal::dirs()->findAllResources("data", appName +"/templates/*/" + desktopName );
		     if ( paths.isEmpty()) {
			   paths = KGlobal::dirs()->findAllResources("data", appName +"/templates/" + desktopName );
	             }
		     if ( paths.isEmpty()) {
		        KMessageBox::error(0L, i18n("No template found for: %1 ").arg(desktopName) );
		        delete shell;
		     } else if ( paths.count() > 1 ) {
		        KMessageBox::error(0L,  i18n("Too many templates found for: %1").arg(desktopName) );
		        delete shell;
		     }
		  }

                  if ( !paths.isEmpty() ) {
		     KUrl templateBase;
		     templateBase.setPath(paths[0]);
		     KDesktopFile templateInfo(paths[0]);

		     QString templateName = templateInfo.readURL();
		     KUrl templateURL;
		     templateURL.setPath( templateBase.directory() + "/" + templateName );
		     if ( shell->openDocument(doc, templateURL )) {
		       doc->resetURL();
		       doc->setEmpty();
                       doc->setTitleModified();
		       kDebug(30003) << "Template loaded..." << endl;
		       n++;
		     } else {
		        KMessageBox::error(0L, i18n("Template %1 failed to load.").arg(templateURL.prettyURL()) );
 		        delete shell;
		     }
		  }
                // now try to load
                } else if ( shell->openDocument( doc, args->url(i) ) ) {
                    if ( print ) {
                        shell->print(false /*we want to get the dialog*/);
                        // delete shell; done by ~KoDocument
                        nPrinted++;
		    } else {
                        // Normal case, success
                        n++;
                    }
                } else {
                    // .... if failed
                    // delete doc; done by openDocument
                    // delete shell; done by ~KoDocument
                }
            }
        }
        if ( print )
            return nPrinted > 0;
        if (n == 0) // no doc, e.g. all URLs were malformed
            return false;
    }

    args->clear();
    // not calling this before since the program will quit there.
    return true;
}

KoApplication::~KoApplication()
{
    delete d->m_appIface;
    delete d;
}

bool KoApplication::isStarting()
{
    return KoApplication::m_starting;
}

#include <KoApplication.moc>
