/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2012 Boudewijn Rempt <boud@valdyas.org>

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
#include "KoAutoSaveRecoveryDialog.h"
#include <KoDpi.h>
#include "KoServiceProvider.h"
#include "KoPart.h"

#include <kdeversion.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kmimetype.h>

#if KDE_IS_VERSION(4,6,0)
#include <krecentdirs.h>
#endif

#include <QtDBus/QtDBus>
#include <QFile>
#include <QSplashScreen>
#include <QSysInfo>

bool KoApplication::m_starting = true;

namespace {
    const QTime appStartTime(QTime::currentTime());
}

class KoApplicationPrivate
{
public:
    KoApplicationPrivate()
        : splashScreen(0)
    {}

    QSplashScreen *splashScreen;
    QList<KoPart *> partList;
};

KoApplication::KoApplication()
        : KApplication(initHack())
        , d(new KoApplicationPrivate)
{
    // Tell the iconloader about share/apps/calligra/icons
    KIconLoader::global()->addAppDir("calligra");

    // Initialize all Calligra directories etc.
    KoGlobal::initialize();

    new KoApplicationAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/application", this);

    m_starting = true;
#ifdef Q_WS_WIN
    QSysInfo::WinVersion version = QSysInfo::windowsVersion();
    printf("setting windows style %i", version);
    switch (version) {
	case QSysInfo::WV_NT:
	case QSysInfo::WV_2000:
            setStyle("windows");
	    break;
	case QSysInfo::WV_XP:
	case QSysInfo::WV_2003:
	    setStyle("windowsxp");
	    break;
	case QSysInfo::WV_VISTA:
	case QSysInfo::WV_WINDOWS7:
	default:
	    setStyle("windowsvista");
    }
#endif

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
    KCmdLineArgs::addCmdLineOptions(options, ki18n("Calligra"), "calligra", "kde");
    return true;
}

// Small helper for start() so that we don't forget to reset m_starting before a return
class KoApplication::ResetStarting
{
public:
    ResetStarting(QSplashScreen *splash = 0)
        : m_splash(splash)
    {
    }

    ~ResetStarting()  {
        KoApplication::m_starting = false;
        if (m_splash) {
            m_splash->hide();
        }
    }

    QSplashScreen *m_splash;
};

bool KoApplication::start()
{
    if (d->splashScreen) {
        d->splashScreen->show();
        d->splashScreen->showMessage(".");
    }

    ResetStarting resetStarting(d->splashScreen); // reset m_starting to false when we're done
    Q_UNUSED(resetStarting);

    // Find the *.desktop file corresponding to the kapp instance name
    KoDocumentEntry entry = KoDocumentEntry(KoServiceProvider::readNativeService());
    if (entry.isEmpty()) {
        kError(30003) << KGlobal::mainComponent().componentName() << "part.desktop not found." << endl;
        kError(30003) << "Run 'kde4-config --path services' to see which directories were searched, assuming kde startup had the same environment as your current shell." << endl;
        kError(30003) << "Check your installation (did you install Calligra in a different prefix than KDE, without adding the prefix to /etc/kderc ?)" << endl;
        return false;
    }

    // Get the command line arguments which we have to parse
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int argsCount = args->count();

    KCmdLineArgs *koargs = KCmdLineArgs::parsedArgs("calligra");
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
#if KDE_IS_VERSION(4,6,0)
        // if there's no document, add the current working directory
        // to the recent dirs so the open dialog and open pane show
        // the directory from where the app was started, instead of
        // the last directory from where we opened a file
        KRecentDirs::add(":OpenDialog", QDir::currentPath());
#endif
        QString errorMsg;
        KoPart *part = entry.createKoPart(&errorMsg);

        if (!part) {
            if (!errorMsg.isEmpty())
                KMessageBox::error(0, errorMsg);
            return false;
        }

        // XXX: the document should be separate plugin
        KoDocument *doc = part->document();

        KoMainWindow *shell = new KoMainWindow(part->componentData());
        shell->show();
        QObject::connect(doc, SIGNAL(sigProgress(int)), shell, SLOT(slotProgress(int)));
        // for initDoc to fill in the recent docs list
        // and for KoDocument::slotStarted
        part->addShell(shell);

        // Check for autosave files from a previous run. There can be several, and
        // we want to offer a restore for every one. Including a nice thumbnail!
        QStringList autoSaveFiles;

        // get all possible autosave files in the home dir, this is for unsaved document autosave files
        // Using the extension allows to avoid relying on the mime magic when opening
        KMimeType::Ptr mime = KMimeType::mimeType(doc->nativeFormatMimeType());
        if (!mime) {
            qFatal("It seems your installation is broken/incomplete cause we failed to load the native mimetype \"%s\".", doc->nativeFormatMimeType().constData());
        }
        QString extension = mime->property("X-KDE-NativeExtension").toString();
        if (extension.isEmpty()) extension = mime->mainExtension();

        QStringList filters;
        filters << QString(".%1-%2-%3-autosave%4").arg(part->componentData().componentName()).arg("*").arg("*").arg(extension);
        QDir dir = QDir::home();

        // all autosave files for our application
        autoSaveFiles = dir.entryList(filters, QDir::Files | QDir::Hidden);

        // all running instances of our application -- bit hackish, but we cannot get at the dbus name here, for some reason
        QDBusReply<QStringList> reply = QDBusConnection::sessionBus().interface()->registeredServiceNames();
        QStringList pids;
        QString ourPid;
        ourPid.setNum(kapp->applicationPid());

        foreach (QString name, reply.value()) {
            if (name.contains(part->componentData().componentName())) {
                // we got another instance of ourselves running, let's get the pid
                QString pid = name.split("-").last();
                if (pid != ourPid) {
                    pids << pid;
                }
            }
        }

        // remove the autosave files that are saved for other, open instances of ourselves
        foreach(const QString &autoSaveFileName, autoSaveFiles) {
            if (!QFile::exists(QDir::homePath() + "/" + autoSaveFileName)) {
                autoSaveFiles.removeAll(autoSaveFileName);
                continue;
            }
            QStringList split = autoSaveFileName.split("-");
            if (split.size() == 4) {
                if (pids.contains(split[1])) {
                    // We've got an active, owned autosave file. Remove.
                    autoSaveFiles.removeAll(autoSaveFileName);
                }
            }
        }

        // Allow the user to make their selection
        if (autoSaveFiles.size() > 0) {
            KoAutoSaveRecoveryDialog dlg(autoSaveFiles);
            if (dlg.exec() == QDialog::Accepted) {
                QStringList filesToRecover = dlg.recoverableFiles();
                foreach (const QString &autosaveFile, autoSaveFiles) {
                    if (!filesToRecover.contains(autosaveFile)) {
                        // remove the files the user didn't want to recover
                        QFile::remove(QDir::homePath() + "/" + autosaveFile);
                    }
                }
                autoSaveFiles = filesToRecover;
            }
            else {
                // don't recover any of the files, but don't delete them either
                autoSaveFiles.clear();
            }
        }

        if (autoSaveFiles.size() > 0) {
            short int numberOfOpenDocuments = 0; // number of documents open
            KUrl url;
            // bah, we need to re-use the document that was already created
            url.setPath(QDir::homePath() + "/" + autoSaveFiles.takeFirst());
            if (shell->openDocument(part, doc, url)) {
                doc->resetURL();
                doc->setModified(true);
                QFile::remove(url.toLocalFile());
                numberOfOpenDocuments++;
            }

            // And then for the other autosave files, we copy & paste the code
            // and loop through them.
            foreach(const QString &autoSaveFile, autoSaveFiles) {
                // For now create an empty document
                QString errorMsg;
                KoPart *part = entry.createKoPart(&errorMsg);
                if (part) {
                    url.setPath(QDir::homePath() + "/" + autoSaveFile);
                    KoMainWindow *shell = new KoMainWindow(part->componentData());
                    shell->show();
                    if (shell->openDocument(part, doc, url)) {
                        doc->resetURL();
                        doc->setModified(true);
                        QFile::remove(url.toLocalFile());
                        numberOfOpenDocuments++;
                    }
                }
            }
            return (numberOfOpenDocuments > 0);
        }
        else {
            part->showStartUpWidget(shell);
        }

    }
    else {
        const bool print = koargs->isSet("print");
        const bool exportAsPdf = koargs->isSet("export-pdf");
        const QString pdfFileName = koargs->getOption("export-filename");
        const QString roundtripFileName = koargs->getOption("roundtrip-filename");
        const bool doTemplate = koargs->isSet("template");
        const bool benchmarkLoading = koargs->isSet("benchmark-loading")
                                      || koargs->isSet("benchmark-loading-show-window")
                                      || !roundtripFileName.isEmpty();
        // only show the shell when no command-line mode option is passed
        const bool showShell =
                koargs->isSet("benchmark-loading-show-window") || (
                    !koargs->isSet("export-pdf")
                    && !koargs->isSet("benchmark-loading")
                    && !koargs->isSet("roundtrip-filename")
                    && roundtripFileName.isEmpty());
        const QString profileFileName = koargs->getOption("profile-filename");
        koargs->clear();

        QTextStream profileoutput;
        QFile profileFile(profileFileName);
        if (!profileFileName.isEmpty()
                && profileFile.open(QFile::WriteOnly | QFile::Truncate)) {
            profileoutput.setDevice(&profileFile);
        }

        // Loop through arguments

        short int numberOfOpenDocuments = 0; // number of documents open
        short int nPrinted = 0;
        for (int argNumber = 0; argNumber < argsCount; argNumber++) {
            // For now create an empty document
            QString errorMsg;
            KoPart *part = entry.createKoPart(&errorMsg);
            if (part) {

                KoDocument *doc = part->document();
                // show a shell asap
                KoMainWindow *shell = new KoMainWindow(part->componentData());
                if (showShell) {
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
                    if (args->url(argNumber).isLocalFile() && QFile::exists(args->url(argNumber).toLocalFile())) {
                        paths << QString(args->url(argNumber).toLocalFile());
                        kDebug(30003) << "using full path...";
                    } else {
                        QString desktopName(args->arg(argNumber));
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
                        if (shell->openDocument(part, doc, templateURL)) {
                            doc->resetURL();
                            doc->setEmpty();
                            doc->setTitleModified();
                            kDebug(30003) << "Template loaded...";
                            numberOfOpenDocuments++;
                        } else {
                            KMessageBox::error(0, i18n("Template %1 failed to load.", templateURL.prettyUrl()));
                            delete shell;
                        }
                    }
                    // now try to load
                }
                else if (shell->openDocument(part, doc, args->url(argNumber))) {
                    if (benchmarkLoading) {
                        if (profileoutput.device()) {
                            profileoutput << "KoApplication::start\t"
                                   << appStartTime.msecsTo(QTime::currentTime())
                                   <<"\t100" << endl;
                        }
                        if (!roundtripFileName.isEmpty()) {
                            part->saveAs(KUrl("file:"+roundtripFileName));
                        }
                        // close the document
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
                        numberOfOpenDocuments++;
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

                d->partList << part;
            }
        }
        if (benchmarkLoading) {
            return false; // no valid urls found.
        }
        if (print || exportAsPdf)
            return nPrinted > 0;
        if (numberOfOpenDocuments == 0) // no doc, e.g. all URLs were malformed
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

void KoApplication::setSplashScreen(QSplashScreen *splashScreen)
{
    d->splashScreen = splashScreen;
}

QList<KoPart*> KoApplication::partList() const
{
    return d->partList;
}

void KoApplication::addPart(KoPart* part)
{
    d->partList << part;
}

#include <KoApplication.moc>
