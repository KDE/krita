/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#include "KoGlobal.h"
#include "KoApplicationAdaptor.h"
#include "KoPrintJob.h"
#include "KoDocumentEntry.h"
#include "KoDocument.h"
#include "KoMainWindow.h"

#include <KoDpi.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <QtDBus/QtDBus>
#include <QFile>

bool KoApplication::m_starting = true;

namespace {
    const QTime appStartTime(QTime::currentTime());
}

class KoApplicationPrivate
{
public:
    KoApplicationPrivate()  {
//         m_appIface = 0;
    }
//     KoApplicationIface *m_appIface;  // to avoid a leak
};

KoApplication::KoApplication()
        : KApplication(initHack())
        , d(new KoApplicationPrivate)
{
    // Tell the iconloader about share/apps/koffice/icons
    KIconLoader::global()->addAppDir("koffice");

    // Initialize all KOffice directories etc.
    KoGlobal::initialize();

    new KoApplicationAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/application", this);

    m_starting = true;
}

// This gets called before entering KApplication::KApplication
bool KoApplication::initHack()
{
    KCmdLineOptions options;
    options.add("print", ki18n("Only print and exit"));
    options.add("template", ki18n("Open a new document with a template"));
    options.add("dpi <dpiX,dpiY>", ki18n("Override display DPI"));
    options.add("export-pdf", ki18n("Only export to PDF and exit"));
    options.add("export-filename <filename>", ki18n("Filename for export-pdf"));
    options.add("benchmark-loading", ki18n("just load the file and then exit"));
    options.add("benchmark-loading-show-window", ki18n("load the file, show the window and progressbar and then exit"));
    options.add("profile-filename <filename>", ki18n("Filename to write profiling information into."));
    options.add("roundtrip-filename <filename>", ki18n("Load a file and save it as an ODF file. Meant for debugging."));
    KCmdLineArgs::addCmdLineOptions(options, ki18n("KOffice"), "koffice", "kde");
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
    Q_UNUSED(resetStarting);

    // Find the *.desktop file corresponding to the kapp instance name
    KoDocumentEntry entry = KoDocumentEntry(KoDocument::readNativeService());
    if (entry.isEmpty()) {
        kError(30003) << KGlobal::mainComponent().componentName() << "part.desktop not found." << endl;
        kError(30003) << "Run 'kde4-config --path services' to see which directories were searched, assuming kde startup had the same environment as your current shell." << endl;
        kError(30003) << "Check your installation (did you install KOffice in a different prefix than KDE, without adding the prefix to /etc/kderc ?)" << endl;
        return false;
    }

    // Get the command line arguments which we have to parse
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int argsCount = args->count();

    KCmdLineArgs *koargs = KCmdLineArgs::parsedArgs("koffice");
    QString dpiValues = koargs->getOption("dpi");
    if (!dpiValues.isEmpty()) {
        int sep = dpiValues.indexOf(QRegExp("[x, ]"));
        int dpiX;
        int dpiY = 0;
        bool ok = true;
        if (sep != -1) {
            dpiY = dpiValues.mid(sep + 1).toInt(&ok);
            dpiValues.truncate(sep);
        }
        if (ok) {
            dpiX = dpiValues.toInt(&ok);
            if (ok) {
                if (!dpiY) dpiY = dpiX;
                KoDpi::setDPI(dpiX, dpiY);
            }
        }
    }

    // No argument -> create an empty document
    if (!argsCount) {
        QString errorMsg;
        KoDocument* doc = entry.createDoc(&errorMsg);
        if (!doc) {
            if (!errorMsg.isEmpty())
                KMessageBox::error(0, errorMsg);
            return false;
        }
        KoMainWindow *shell = new KoMainWindow(doc->componentData());
        shell->show();
        QObject::connect(doc, SIGNAL(sigProgress(int)), shell, SLOT(slotProgress(int)));
        // for initDoc to fill in the recent docs list
        // and for KoDocument::slotStarted
        doc->addShell(shell);

        if (doc->checkAutoSaveFile()) {
            shell->setRootDocument(doc);
        } else {
            doc->showStartUpWidget(shell);
        }

        // FIXME This needs to be moved someplace else
        QObject::disconnect(doc, SIGNAL(sigProgress(int)), shell, SLOT(slotProgress(int)));
    } else {
        const bool print = koargs->isSet("print");
        const bool exportAsPdf = koargs->isSet("export-pdf");
        const QString pdfFileName = koargs->getOption("export-filename");
        const QString roundtripFileName = koargs->getOption("roundtrip-filename");
        const bool doTemplate = koargs->isSet("template");
        const bool benchmarkLoading = koargs->isSet("benchmark-loading")
                                      || koargs->isSet("benchmark-loading-show-window")
                                      || !roundtripFileName.isEmpty();
        const bool showShell = koargs->isSet("benchmark-loading-show-window");
        const QString profileFileName = koargs->getOption("profile-filename");
        koargs->clear();

        QTextStream profileoutput;
        QFile profileFile(profileFileName);
        if (!profileFileName.isEmpty()
                && profileFile.open(QFile::WriteOnly | QFile::Truncate)) {
            profileoutput.setDevice(&profileFile);
        }

        // Loop through arguments

        short int n = 0; // number of documents open
        short int nPrinted = 0;
        for (int i = 0; i < argsCount; i++) {
            // For now create an empty document
            QString errorMsg;
            KoDocument* doc = entry.createDoc(&errorMsg, 0);
            if (doc) {
                // show a shell asap
                KoMainWindow *shell = new KoMainWindow(doc->componentData());
                if (showShell || !benchmarkLoading) {
                    shell->show();
                }
                if (benchmarkLoading) {
                    doc->setReadWrite(false);
                }

                if (profileoutput.device()) {
                    doc->setProfileStream(&profileoutput);
                    profileoutput << "KoApplication::start\t"
                            << appStartTime.msecsTo(QTime::currentTime())
                            <<"\t0" << endl;
                    doc->setAutoErrorHandlingEnabled(false);
                }
                doc->setProfileReferenceTime(appStartTime);

                // are we just trying to open a template?
                if (doTemplate) {
                    QStringList paths;
                    if (args->url(i).isLocalFile() && QFile::exists(args->url(i).toLocalFile())) {
                        paths << QString(args->url(i).toLocalFile());
                        kDebug(30003) << "using full path...";
                    } else {
                        QString desktopName(args->arg(i));
                        QString appName = KGlobal::mainComponent().componentName();

                        paths = KGlobal::dirs()->findAllResources("data", appName + "/templates/*/" + desktopName);
                        if (paths.isEmpty()) {
                            paths = KGlobal::dirs()->findAllResources("data", appName + "/templates/" + desktopName);
                        }
                        if (paths.isEmpty()) {
                            KMessageBox::error(0, i18n("No template found for: %1", desktopName));
                            delete shell;
                        } else if (paths.count() > 1) {
                            KMessageBox::error(0, i18n("Too many templates found for: %1", desktopName));
                            delete shell;
                        }
                    }

                    if (!paths.isEmpty()) {
                        KUrl templateBase;
                        templateBase.setPath(paths[0]);
                        KDesktopFile templateInfo(paths[0]);

                        QString templateName = templateInfo.readUrl();
                        KUrl templateURL;
                        templateURL.setPath(templateBase.directory() + '/' + templateName);
                        if (shell->openDocument(doc, templateURL)) {
                            doc->resetURL();
                            doc->setEmpty();
                            doc->setTitleModified();
                            kDebug(30003) << "Template loaded...";
                            n++;
                        } else {
                            KMessageBox::error(0, i18n("Template %1 failed to load.", templateURL.prettyUrl()));
                            delete shell;
                        }
                    }
                    // now try to load
                }
                else if (shell->openDocument(doc, args->url(i))) {
                    if (benchmarkLoading) {
                        if (profileoutput.device()) {
                            profileoutput << "KoApplication::start\t"
                                   << appStartTime.msecsTo(QTime::currentTime())
                                   <<"\t100" << endl;
                        }
                        if (!roundtripFileName.isEmpty()) {
                            doc->saveAs(KUrl("file:"+roundtripFileName));
                        }
                        shell->slotFileQuit();
                        return true; // only load one document!
                    }
                    else if (print) {
                        shell->slotFilePrint();
                        // delete shell; done by ~KoDocument
                        nPrinted++;
                    } else if (exportAsPdf) {
                        KoPrintJob *job = shell->exportToPdf(pdfFileName);
                        if (job)
                            connect (job, SIGNAL(destroyed(QObject*)), shell,
                                    SLOT(slotFileQuit()), Qt::QueuedConnection);
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

                if (profileoutput.device()) {
                    profileoutput << "KoApplication::start\t"
                            << appStartTime.msecsTo(QTime::currentTime())
                            <<"\t100" << endl;
                }
            }
        }
        if (benchmarkLoading) {
            return false; // no valid urls found.
        }
        if (print || exportAsPdf)
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
//     delete d->m_appIface;
    delete d;
}

bool KoApplication::isStarting()
{
    return KoApplication::m_starting;
}

#include <KoApplication.moc>
