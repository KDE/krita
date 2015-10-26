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

#include "KisApplication.h"

#include <stdlib.h>
#ifdef Q_OS_WIN
#include <windows.h>
#include <tchar.h>
#endif

#include <QMessageBox>
#include <QFile>
#include <QWidget>
#include <QSysInfo>
#include <QStringList>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QDir>
#include <QDesktopWidget>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTimer>
#include <QStyle>
#include <QStyleFactory>

#include <klocalizedstring.h>
#include <kdesktopfile.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <KoPluginLoader.h>
#include <KoShapeRegistry.h>
#include <KoDpi.h>
#include "KoGlobal.h"
#include "KoConfig.h"
#include <KoHashGeneratorProvider.h>
#include <KoResourcePaths.h>

#include "KisPrintJob.h"
#include "KisDocumentEntry.h"
#include "KisDocument.h"
#include "KisMainWindow.h"
#include "KisAutoSaveRecoveryDialog.h"
#include "KisPart.h"
#include <kis_icon.h>
#include "kis_md5_generator.h"
#include "kis_config.h"
#include "flake/kis_shape_selection.h"
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>
#include <generator/kis_generator.h>
#include <kis_paintop_registry.h>
#include <metadata/kis_meta_data_io_backend.h>
#include "kisexiv2/kis_exiv2.h"
#include "KisApplicationArguments.h"
#include <kis_debug.h>

#ifdef HAVE_OPENGL
#include "opengl/kis_opengl.h"
#endif

#include <CalligraVersionWrapper.h>

namespace {
const QTime appStartTime(QTime::currentTime());
}

class KisApplicationPrivate
{
public:
    KisApplicationPrivate()
        : splashScreen(0)
    {}
    QWidget *splashScreen;
};

class KisApplication::ResetStarting
{
public:
    ResetStarting(QWidget *splash = 0)
        : m_splash(splash)
    {
    }

    ~ResetStarting()  {
        if (m_splash) {

            KConfigGroup cfg( KSharedConfig::openConfig(), "SplashScreen");
            bool hideSplash = cfg.readEntry("HideSplashAfterStartup", false);

            if (hideSplash) {
                m_splash->hide();
            }
            else {
                m_splash->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                QRect r(QPoint(), m_splash->size());
                m_splash->move(QApplication::desktop()->availableGeometry().center() - r.center());
                m_splash->setWindowTitle(qAppName());
                foreach(QObject *o, m_splash->children()) {
                    QWidget *w = qobject_cast<QWidget*>(o);
                    if (w && w->isHidden()) {
                        w->setVisible(true);
                    }
                }

                m_splash->show();
            }
        }
    }

    QWidget *m_splash;
};



KisApplication::KisApplication(const QString &key, int &argc, char **argv)
    : QtSingleApplication(key, argc, argv)
    , d(new KisApplicationPrivate)
{
    setApplicationDisplayName("Krita");
    setApplicationName("krita");

    QString version = CalligraVersionWrapper::versionString(true);
    setApplicationVersion(version);

    // Initialize all Calligra directories etc.
    KoGlobal::initialize();

    // for cursors
    KoResourcePaths::addResourceType("kis_pics", "data", "krita/pics/");

    // for images in the paintop box
    KoResourcePaths::addResourceType("kis_images", "data", "krita/images/");

    KoResourcePaths::addResourceType("icc_profiles", "data", "krita/profiles/");

    setWindowIcon(KisIconUtils::loadIcon("calligrakrita"));

#ifdef HAVE_OPENGL
    KisOpenGL::initialize();
#endif

#ifdef Q_OS_MACX
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
    setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    qDebug() << "Available styles:" << QStyleFactory::keys();
    QStringList styles = QStringList() /*<< "Breeze"*/ << "Oxygen" << "Plastique" << "Fusion";
    foreach(const QString & style, styles) {
        if (!setStyle(style)) {
            qDebug() << "No" << style << "available.";
        }
        else {
            break;
        }
    }
    qDebug() << "Style:" << QApplication::style();



    KoHashGeneratorProvider::instance()->setGenerator("MD5", new KisMD5Generator());
}

#if defined(Q_OS_WIN) && defined(ENV32BIT)
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL isWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
                GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}
#endif


bool KisApplication::createNewDocFromTemplate(const QString &fileName, KisMainWindow *mainWindow)
{
    QString templatePath;

    const QUrl templateUrl = QUrl::fromLocalFile(fileName);
    if (QFile::exists(fileName)) {
        templatePath = templateUrl.toLocalFile();
        dbgUI << "using full path...";
    }
    else {
        QString desktopName(fileName);
        const QString templatesResourcePath = KisPart::instance()->templatesResourcePath();

        QStringList paths = KoResourcePaths::findAllResources("data", templatesResourcePath + "*/" + desktopName);
        if (paths.isEmpty()) {
            paths = KoResourcePaths::findAllResources("data", templatesResourcePath + desktopName);
        }

        if (paths.isEmpty()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("No template found for: %1", desktopName));
        } else if (paths.count() > 1) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("Too many templates found for: %1", desktopName));
        } else {
            templatePath = paths.at(0);
        }
    }

    if (!templatePath.isEmpty()) {
        QUrl templateBase;
        templateBase.setPath(templatePath);
        KDesktopFile templateInfo(templatePath);

        QString templateName = templateInfo.readUrl();
        QUrl templateURL;
        templateURL.setPath(templateBase.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path() + '/' + templateName);

        KisDocument *doc = KisPart::instance()->createDocument();
        if (mainWindow->openDocumentInternal(templateURL, doc)) {
            doc->resetURL();
            doc->setEmpty();
            doc->setTitleModified();
            dbgUI << "Template loaded...";
            return true;
        }
        else {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("Template %1 failed to load.", templateURL.toDisplayString()));
        }
    }

    return false;
}

void KisApplication::clearConfig()
{
    KIS_ASSERT_RECOVER_RETURN(qApp->thread() == QThread::currentThread());

    KSharedConfigPtr config =  KSharedConfig::openConfig();

    // find user settings file
    bool createDir = false;
    QString kritarcPath = KoResourcePaths::locateLocal("config", "kritarc", createDir);

    QFile configFile(kritarcPath);
    if (configFile.exists()) {
        // clear file
        if (configFile.open(QFile::WriteOnly)) {
            configFile.close();
        }
        else {
            QMessageBox::warning(0,
                                 i18nc("@title:window", "Krita"),
                                 i18n("Failed to clear %1\n\n"
                                      "Please make sure no other program is using the file and try again.",
                                      kritarcPath),
                                 QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    // reload from disk; with the user file settings cleared,
    // this should load any default configuration files shipping with the program
    config->reparseConfiguration();
    config->sync();
}

void KisApplication::askClearConfig()
{
    Qt::KeyboardModifiers mods = QApplication::queryKeyboardModifiers();
    bool askClearConfig = (mods & Qt::ControlModifier) && (mods & Qt::ShiftModifier) && (mods & Qt::AltModifier);

    if (askClearConfig) {
        bool ok = QMessageBox::question(0,
                                        i18nc("@title:window", "Krita"),
                                        i18n("Do you want to clear the settings file?"),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
        if (ok) {
            clearConfig();
        }
    }
}

bool KisApplication::start(const KisApplicationArguments &args)
{

#if defined(Q_OS_WIN)  || defined (Q_OS_MACX)
#ifdef ENV32BIT
    KisConfig cfg;
    if (isWow64() && !cfg.readEntry("WarnedAbout32Bits", false)) {
        QMessageBox::information(0,
                                 i18nc("@title:window", "Krita: Warning"),
                                 i18n("You are running a 32 bits build on a 64 bits Windows.\n"
                                      "This is not recommended.\n"
                                      "Please download and install the x64 build instead."));
        cfg.writeEntry("WarnedAbout32Bits", true);

    }
#endif

    QDir appdir(applicationDirPath());
    appdir.cdUp();

    //KoResourcePaths::addXdgDataPrefix(appdir.absolutePath() + "/share");
    //KoResourcePaths::addPrefix(appdir.absolutePath());

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // If there's no kdehome, set it and restart the process.
    if (!env.contains("KDEHOME")) {
        qputenv("KDEHOME", QFile::encodeName(QDesktopServices::storageLocation(QDesktopServices::DataLocation)));
    }
    if (!env.contains("XDG_DATA_DIRS")) {
        qputenv("XDG_DATA_DIRS", QFile::encodeName(appdir.absolutePath() + "/share"));
    }
    if (!env.contains("KDEDIR")) {
        qputenv("KDEDIR", QFile::encodeName(appdir.absolutePath()));
    }
    if (!env.contains("KDEDIRS")) {
        qputenv("KDEDIRS", QFile::encodeName(appdir.absolutePath()));
    }
    qputenv("PATH", QFile::encodeName(appdir.absolutePath() + "/bin" + ";"
                                      + appdir.absolutePath() + "/lib" + ";"
                                      + appdir.absolutePath() + "/lib/kde4" + ";"
                                      + appdir.absolutePath() + "/Frameworks" + ";"
                                      + appdir.absolutePath()));

#endif

    int dpiX = args.dpiX();
    int dpiY = args.dpiY();
    if (dpiX > 0 && dpiY > 0) {
        KoDpi::setDPI(dpiX, dpiY);
    }

    const bool doTemplate = args.doTemplate();
    const bool print = args.print();
    const bool exportAs = args.exportAs();
    const bool exportAsPdf = args.exportAsPdf();
    const QString exportFileName = args.exportFileName();
    const QString profileFileName = args.profileFileName();

    const bool batchRun = (print || exportAs || exportAsPdf);
    // print & exportAsPdf do user interaction ATM
    const bool needsMainWindow = !exportAs;
    // only show the mainWindow when no command-line mode option is passed
    // TODO: fix print & exportAsPdf to work without mainwindow shown
    const bool showmainWindow = !exportAs; // would be !batchRun;
    const bool runningInGnome = (qgetenv("XDG_CURRENT_DESKTOP") == "GNOME");
    const bool showSplashScreen = !batchRun && !runningInGnome;

    if (d->splashScreen && showSplashScreen) {
        d->splashScreen->show();
        d->splashScreen->repaint();
        processEvents();
    }

    ResetStarting resetStarting(d->splashScreen); // remove the splash when done
    Q_UNUSED(resetStarting);

    // Load various global plugins
    KoShapeRegistry* r = KoShapeRegistry::instance();
    r->add(new KisShapeSelectionFactory());

    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
    KisPaintOpRegistry::instance();

    // Load the krita-specific tools
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Tool"),
                                     QString::fromLatin1("[X-Krita-Version] == 28"));

    // Load dockers
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Dock"),
                                     QString::fromLatin1("[X-Krita-Version] == 28"));


    // XXX_EXIV: make the exiv io backends real plugins
    KisExiv2::initialize();

    KisMainWindow *mainWindow = 0;

    if (needsMainWindow) {
        // show a mainWindow asap, if we want that
        mainWindow = KisPart::instance()->createMainWindow();

        if (showmainWindow) {
            QTimer::singleShot(1, mainWindow, SLOT(show()));
        }
    }
    short int numberOfOpenDocuments = 0; // number of documents open


    // Check for autosave files that can be restored, if we're not running a batchrun (test, print, export to pdf)
    QList<QUrl> urls = checkAutosaveFiles();
    if (!batchRun && mainWindow) {
        foreach(const QUrl &url, urls) {
            KisDocument *doc = KisPart::instance()->createDocument();
            mainWindow->openDocumentInternal(url, doc);
        }
    }

    // Get the command line arguments which we have to parse
    int argsCount = args.filenames().count();
    if (argsCount > 0) {

        QTextStream profileoutput;
        QFile profileFile(profileFileName);
        if (!profileFileName.isEmpty()
                && profileFile.open(QFile::WriteOnly | QFile::Truncate)) {
            profileoutput.setDevice(&profileFile);
        }


        // Loop through arguments
        short int nPrinted = 0;
        for (int argNumber = 0; argNumber < argsCount; argNumber++) {
            QString fileName = args.filenames().at(argNumber);
            // are we just trying to open a template?
            if (doTemplate) {
                // called in mix with batch options? ignore and silently skip
                if (batchRun) {
                    continue;
                }
                if (createNewDocFromTemplate(fileName, mainWindow)) {
                    ++numberOfOpenDocuments;
                }
            // now try to load
            }
            else {

                if (exportAs) {
                    QMimeType outputMimetype;
                    QMimeDatabase db;
                    outputMimetype = db.mimeTypeForFile(exportFileName);
                    if (outputMimetype.isDefault()) {
                        dbgKrita << i18n("Mimetype not found, try using the -mimetype option") << endl;
                        return 1;
                    }

                    QApplication::setOverrideCursor(Qt::WaitCursor);

                    QString outputFormat = outputMimetype.name();

                    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;
                    KisImportExportManager manager(fileName);
                    manager.setBatchMode(true);
                    QByteArray mime(outputFormat.toLatin1());
                    status = manager.exportDocument(exportFileName, mime);

                    if (status != KisImportExportFilter::OK) {
                        dbgKrita << "Could not export " << fileName << "to" << exportFileName << ":" << (int)status;
                    }
                    nPrinted++;
                    QTimer::singleShot(0, this, SLOT(quit()));
                }
                else if (mainWindow) {
                    KisDocument *doc = KisPart::instance()->createDocument();
                    if (mainWindow->openDocumentInternal(QUrl::fromLocalFile(fileName), doc)) {
                        if (print) {
                            mainWindow->slotFilePrint();
                            nPrinted++;
                            // TODO: trigger closing of app once printing is done
                        }
                        else if (exportAsPdf) {
                            KisPrintJob *job = mainWindow->exportToPdf(exportFileName);
                            if (job)
                                connect (job, SIGNAL(destroyed(QObject*)), mainWindow,
                                        SLOT(slotFileQuit()), Qt::QueuedConnection);
                            nPrinted++;
                        } else {
                            // Normal case, success
                            numberOfOpenDocuments++;
                        }
                    } else {
                    // .... if failed
                    // delete doc; done by openDocument
                    }
                }
            }
        }

        if (batchRun) {
            return nPrinted > 0;
        }
    }
    // not calling this before since the program will quit there.
    return true;
}

KisApplication::~KisApplication()
{
    delete d;
}

void KisApplication::setSplashScreen(QWidget *splashScreen)
{
    d->splashScreen = splashScreen;
}

bool KisApplication::notify(QObject *receiver, QEvent *event)
{
    try {
        return QApplication::notify(receiver, event);
    } catch (std::exception &e) {
        qWarning("Error %s sending event %i to object %s",
                 e.what(), event->type(), qPrintable(receiver->objectName()));
    } catch (...) {
        qWarning("Error <unknown> sending event %i to object %s",
                 event->type(), qPrintable(receiver->objectName()));
    }
    return false;

}


void KisApplication::remoteArguments(QByteArray message, QObject *socket)
{
    Q_UNUSED(socket);

    // check if we have any mainwindow
    KisMainWindow *mw = qobject_cast<KisMainWindow*>(qApp->activeWindow());
    if (!mw) {
        mw = KisPart::instance()->mainWindows().first();
    }

    if (!mw) {
        return;
    }

    KisApplicationArguments args = KisApplicationArguments::deserialize(message);
    const bool doTemplate = args.doTemplate();
    const int argsCount = args.filenames().count();

    if (argsCount > 0) {
        // Loop through arguments
        for (int argNumber = 0; argNumber < argsCount; ++argNumber) {
            QString filename = args.filenames().at(argNumber);
            // are we just trying to open a template?
            if (doTemplate) {
                createNewDocFromTemplate(filename, mw);
            }
            else if (QFile(filename).exists()) {
                KisDocument *doc = KisPart::instance()->createDocument();
                mw->openDocumentInternal(QUrl::fromLocalFile(filename), doc);
            }
        }
    }
}

void KisApplication::fileOpenRequested(const QString &url)
{
    KisMainWindow *mainWindow = KisPart::instance()->mainWindows().first();
    if (mainWindow) {
        KisDocument *doc = KisPart::instance()->createDocument();
        mainWindow->openDocumentInternal(QUrl::fromLocalFile(url), doc);
    }
}


QList<QUrl> KisApplication::checkAutosaveFiles()
{
    // Check for autosave files from a previous run. There can be several, and
    // we want to offer a restore for every one. Including a nice thumbnail!
    QStringList autoSaveFiles;

    QStringList filters;
    filters << QString(".krita-*-*-autosave.kra");

#ifdef Q_OS_WIN
    QDir dir = QDir::temp();
#else
    QDir dir = QDir::home();
#endif

    // all autosave files for our application
    autoSaveFiles = dir.entryList(filters, QDir::Files | QDir::Hidden);

    // Allow the user to make their selection
    if (autoSaveFiles.size() > 0) {
        KisAutoSaveRecoveryDialog *dlg = new KisAutoSaveRecoveryDialog(autoSaveFiles, activeWindow());
        if (dlg->exec() == QDialog::Accepted) {

            QStringList filesToRecover = dlg->recoverableFiles();
            foreach (const QString &autosaveFile, autoSaveFiles) {
                if (!filesToRecover.contains(autosaveFile)) {
                    QFile::remove(dir.absolutePath() + "/" + autosaveFile);
                }
            }
            autoSaveFiles = filesToRecover;
        }
        else {
            // don't recover any of the files, but don't delete them either
            autoSaveFiles.clear();
        }
    }

    QList<QUrl> autosaveUrls;
    if (autoSaveFiles.size() > 0) {

        foreach(const QString &autoSaveFile, autoSaveFiles) {
            const QUrl url = QUrl::fromLocalFile(dir.absolutePath() + QLatin1Char('/') + autoSaveFile);
            autosaveUrls << url;
        }
    }

    return autosaveUrls;
}

