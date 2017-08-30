/*
 * Copyright (c) 1999 Matthias Elter <me@kde.org>
 * Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 * Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>

#include <QString>
#include <QPixmap>
#include <kis_debug.h>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QDir>
#include <QDate>
#include <QLocale>
#include <QSettings>

#include <time.h>

#include <KisApplication.h>
#include <KoConfig.h>
#include <KoResourcePaths.h>

#include "data/splash/splash_screen.xpm"
#include "data/splash/splash_holidays.xpm"
#include "KisDocument.h"
#include "kis_splash_screen.h"
#include "KisPart.h"
#include "KisApplicationArguments.h"
#include <opengl/kis_opengl.h>
#include "input/KisQtWidgetsTweaker.h"

#if defined Q_OS_WIN
#include <windows.h>
#include <stdlib.h>
#include <kis_tablet_support_win.h>
#include <kis_tablet_support_win8.h>
#include <kis_config.h>

#elif defined HAVE_X11
#include <kis_tablet_support_x11.h>
#include <kis_xi2_event_filter.h>
#endif

#if defined HAVE_KCRASH
#include <kcrash.h>
#elif defined USE_DRMINGW
namespace
{
void tryInitDrMingw()
{
    wchar_t path[MAX_PATH];
    QString pathStr = QCoreApplication::applicationDirPath().replace(L'/', L'\\') + QStringLiteral("\\exchndl.dll");
    if (pathStr.size() > MAX_PATH - 1) {
        return;
    }
    int pathLen = pathStr.toWCharArray(path);
    path[pathLen] = L'\0'; // toWCharArray doesn't add NULL terminator
    HMODULE hMod = LoadLibraryW(path);
    if (!hMod) {
        return;
    }
    // No need to call ExcHndlInit since the crash handler is installed on DllMain
    auto myExcHndlSetLogFileNameA = reinterpret_cast<BOOL (APIENTRY *)(const char *)>(GetProcAddress(hMod, "ExcHndlSetLogFileNameA"));
    if (!myExcHndlSetLogFileNameA) {
        return;
    }
    // Set the log file path to %LocalAppData%\kritacrash.log
    QString logFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).replace(L'/', L'\\') + QStringLiteral("\\kritacrash.log");
    myExcHndlSetLogFileNameA(logFile.toLocal8Bit());
}
} // namespace
#endif
extern "C" int main(int argc, char **argv)
{

    // The global initialization of the random generator
    qsrand(time(0));
    bool runningInKDE = !qgetenv("KDE_FULL_SESSION").isEmpty();

#if defined HAVE_X11
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif


    // A per-user unique string, without /, because QLocalServer cannot use names with a / in it
    QString key = "Krita3" + QDesktopServices::storageLocation(QDesktopServices::HomeLocation).replace("/", "_");
    key = key.replace(":", "_").replace("\\","_");

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    bool singleApplication = true;
    bool enableOpenGLDebug = false;
    bool openGLDebugSynchronous = false;
    {
        QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
        singleApplication = kritarc.value("EnableSingleApplication", true).toBool();
#if QT_VERSION >= 0x050600
        if (kritarc.value("EnableHiDPI", false).toBool()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
        if (!qgetenv("KRITA_HIDPI").isEmpty()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
#endif
        if (!qgetenv("KRITA_OPENGL_DEBUG").isEmpty()) {
            enableOpenGLDebug = true;
        } else {
            enableOpenGLDebug = kritarc.value("EnableOpenGLDebug", false).toBool();
        }
        if (enableOpenGLDebug && (qgetenv("KRITA_OPENGL_DEBUG") == "sync" || kritarc.value("OpenGLDebugSynchronous", false).toBool())) {
            openGLDebugSynchronous = true;
        }
    }
    KisOpenGL::setDefaultFormat(enableOpenGLDebug, openGLDebugSynchronous);

    KLocalizedString::setApplicationDomain("krita");

    // first create the application so we can create a pixmap
    KisApplication app(key, argc, argv);

#ifdef Q_OS_LINUX
    qputenv("XDG_DATA_DIRS", QFile::encodeName(KoResourcePaths::getApplicationRoot() + "share") + ":" + qgetenv("XDG_DATA_DIRS"));
#else
    qputenv("XDG_DATA_DIRS", QFile::encodeName(KoResourcePaths::getApplicationRoot() + "share"));
#endif

    qDebug() << "Setting XDG_DATA_DIRS" << qgetenv("XDG_DATA_DIRS");
    qDebug() << "Available translations" << KLocalizedString::availableApplicationTranslations();
    qDebug() << "Available domain translations" << KLocalizedString::availableDomainTranslations("krita");

    // Now that the paths are set, set the language. First check the override from the langage
    // selection dialog.
    {
        QSettings languageoverride(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat);
        languageoverride.beginGroup(QStringLiteral("Language"));
        QString language = languageoverride.value(qAppName(), "").toString();

        qDebug() << "Override language:" << language;

        if (!language.isEmpty()) {
            KLocalizedString::setLanguages(language.split(":"));
            // And override Qt's locale, too
            qputenv("LANG", language.split(":").first().toUtf8());
            QLocale locale(language.split(":").first());
            QLocale::setDefault(locale);
            qDebug() << "Qt ui languages" << locale.uiLanguages();
        }
        else {
            // And if there isn't one, check the one set by the system.
            // XXX: This doesn't work, for some !@#$% reason.
            QLocale locale = QLocale::system();
            if (locale.bcp47Name() != QStringLiteral("en")) {
                qputenv("LANG", locale.bcp47Name().toLatin1());
                KLocalizedString::setLanguages(QStringList() << locale.bcp47Name());
            }
        }

    }
#ifdef Q_OS_WIN
    QDir appdir(KoResourcePaths::getApplicationRoot());
    QString path = qgetenv("PATH");
    qputenv("PATH", QFile::encodeName(appdir.absolutePath() + "/bin" + ";"
                                      + appdir.absolutePath() + "/lib" + ";"
                                      + appdir.absolutePath() + "/Frameworks" + ";"
                                      + appdir.absolutePath() + ";"
                                      + path));

    qDebug() << "PATH" << qgetenv("PATH");
#endif

    if (qApp->applicationDirPath().contains(KRITA_BUILD_DIR)) {
        qFatal("FATAL: You're trying to run krita from the build location. You can only run Krita from the installation location.");
    }


#if defined HAVE_KCRASH
    KCrash::initialize();
#elif defined USE_DRMINGW
    tryInitDrMingw();
#endif

    // If we should clear the config, it has to be done as soon as possible after
    // KisApplication has been created. Otherwise the config file may have been read
    // and stored in a KConfig object we have no control over.
    app.askClearConfig();

    KisApplicationArguments args(app);

    if (singleApplication && app.isRunning()) {
        // only pass arguments to main instance if they are not for batch processing
        // any batch processing would be done in this separate instance
        const bool batchRun = (args.print() || args.exportAs() || args.exportAsPdf());

        if (!batchRun) {
            QByteArray ba = args.serialize();
            if (app.sendMessage(ba)) {
                return 0;
            }
        }
    }

    if (!runningInKDE) {
        // Icons in menus are ugly and distracting
        app.setAttribute(Qt::AA_DontShowIconsInMenus);
    }

#if defined HAVE_X11
    app.installNativeEventFilter(KisXi2EventFilter::instance());
#endif
    app.installEventFilter(KisQtWidgetsTweaker::instance());


    if (!args.noSplash()) {
        // then create the pixmap from an xpm: we cannot get the
        // location of our datadir before we've started our components,
        // so use an xpm.
        QDate currentDate = QDate::currentDate();
        QWidget *splash = 0;
        if (currentDate > QDate(currentDate.year(), 12, 4) ||
                currentDate < QDate(currentDate.year(), 1, 9)) {
            splash = new KisSplashScreen(app.applicationVersion(), QPixmap(splash_holidays_xpm));
        }
        else {
            splash = new KisSplashScreen(app.applicationVersion(), QPixmap(splash_screen_xpm));
        }

        app.setSplashScreen(splash);
    }


#if defined Q_OS_WIN
    {
        KisConfig cfg;
        bool isUsingWin8PointerInput = false;
        if (cfg.useWin8PointerInput()) {
            KisTabletSupportWin8 *penFilter = new KisTabletSupportWin8();
            if (penFilter->init()) {
                // penFilter.registerPointerDeviceNotifications();
                app.installNativeEventFilter(penFilter);
                isUsingWin8PointerInput = true;
                qDebug() << "Using Win8 Pointer Input for tablet support";
            } else {
                qDebug() << "No Win8 Pointer Input available";
                delete penFilter;
            }
        }
        if (!isUsingWin8PointerInput) {
            KisTabletSupportWin::init();
            // app.installNativeEventFilter(new KisTabletSupportWin());
        }
    }
#endif

    if (!app.start(args)) {
        return 1;
    }

#if QT_VERSION >= 0x050700
    app.setAttribute(Qt::AA_CompressHighFrequencyEvents, false);
#endif

    // Set up remote arguments.
    QObject::connect(&app, SIGNAL(messageReceived(QByteArray,QObject*)),
                     &app, SLOT(remoteArguments(QByteArray,QObject*)));

    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)),
                     &app, SLOT(fileOpenRequested(QString)));

    int state = app.exec();

    {
        QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
        kritarc.setValue("canvasState", "OPENGL_SUCCESS");
    }

    return state;
}

