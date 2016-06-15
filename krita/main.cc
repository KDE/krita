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
#include <QLoggingCategory>
#include <QLocale>
#include <QSettings>

#include <KisApplication.h>
#include <KoConfig.h>
#include <KoResourcePaths.h>

#include "data/splash/splash_screen.xpm"
#include "data/splash/splash_holidays.xpm"
#include "KisDocument.h"
#include "kis_splash_screen.h"
#include "KisPart.h"
#include "KisApplicationArguments.h"

#if defined Q_OS_WIN
#include <windows.h>
#include <stdlib.h>
#include <kis_tablet_support_win.h>

#elif defined HAVE_X11
    #include <kis_tablet_support_x11.h>
    #include <kis_xi2_event_filter.h>
#endif

#if defined HAVE_KCRASH
#include <kcrash.h>

#elif defined USE_BREAKPAD
    #include "kis_crash_handler.h"
#endif
extern "C" int main(int argc, char **argv)
{
    /**
     * Add a workaround for Qt 5.6, which implemented compression of the tablet events.
     * Since Qt 5.6.1 there will be this hacky environment variable option. After that,
     * Qt developers promised to give us better control for that. Please make sure the env
     * variable is set *before* the construction of QApplication!
     */
#if defined Q_OS_LINUX && QT_VERSION >= 0x050600
    qputenv("QT_XCB_NO_EVENT_COMPRESSION", "1");
#endif

    bool runningInKDE = !qgetenv("KDE_FULL_SESSION").isEmpty();

    /**
     * Disable debug output by default. (krita.input enables tablet debugging.)
     * Debug logs can be controlled by an environment variable QT_LOGGING_RULES.
     *
     * As an example, to get full debug output, run the following:
     * export QT_LOGGING_RULES="krita*=true"; krita
     *
     * See: http://doc.qt.io/qt-5/qloggingcategory.html
     */
    QLoggingCategory::setFilterRules("krita*.debug=false\n"
                                     "krita*.warning=true\n"
                                     "krita.tabletlog=true");

    // A per-user unique string, without /, because QLocalServer cannot use names with a / in it
    QString key = "Krita3" + QDesktopServices::storageLocation(QDesktopServices::HomeLocation).replace("/", "_");
    key = key.replace(":", "_").replace("\\","_");

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

#if QT_VERSION >= 0x050600
    if (!qgetenv("KRITA_HIDPI").isEmpty()) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }
#endif

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
    KLocalizedString::clearLanguages();
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings languageoverride(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat);
    languageoverride.beginGroup(QStringLiteral("Language"));
    QString language = languageoverride.value(qAppName(), "").toString();

    qDebug() << "Override language:" << language;

    if (!language.isEmpty()) {
        KLocalizedString::setLanguages(language.split(":"));
        // And override Qt's locale, too
        qputenv("LANG", language.toLatin1());
        QLocale locale(language.split(":").first());
        QLocale::setDefault(locale);
        qDebug() << "Qt ui languages" << locale.uiLanguages();
    }
    else {
        // And if there isn't one, check the one set by the system.
        // XXX: This doesn't work, for some !@#$% reason.
        QLocale locale = QLocale::system();
        if (locale.bcp47Name() != QStringLiteral("en")) {
            KLocalizedString::setLanguages(QStringList() << locale.bcp47Name());
        }
    }

#ifdef Q_OS_WIN
    QDir appdir(KoResourcePaths::getApplicationRoot());
    qputenv("PATH", QFile::encodeName(appdir.absolutePath() + "/bin" + ";"
                                      + appdir.absolutePath() + "/lib" + ";"
                                      + appdir.absolutePath() + "/lib/kde4" + ";"
                                      + appdir.absolutePath() + "/Frameworks" + ";"
                                      + appdir.absolutePath()));

    qDebug() << "PATH" << qgetenv("PATH");
#endif

    if (qApp->applicationDirPath().contains(KRITA_BUILD_DIR)) {
        qFatal("FATAL: You're trying to run krita from the build location. You can only run Krita from the installation location.");
    }


#if defined HAVE_KCRASH
    KCrash::initialize();
#elif defined USE_BREAKPAD
    qputenv("KDE_DEBUG", "1");
    KisCrashHandler crashHandler;
    Q_UNUSED(crashHandler);
#endif

    // If we should clear the config, it has to be done as soon as possible after
    // KisApplication has been created. Otherwise the config file may have been read
    // and stored in a KConfig object we have no control over.
    app.askClearConfig();

    KisApplicationArguments args(app);

    if (app.isRunning()) {
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


#if defined Q_OS_WIN
    KisTabletSupportWin::init();
    // app.installNativeEventFilter(new KisTabletSupportWin());
#endif

    if (!app.start(args)) {
        return 1;
    }

    // Set up remote arguments.
    QObject::connect(&app, SIGNAL(messageReceived(QByteArray,QObject*)),
                     &app, SLOT(remoteArguments(QByteArray,QObject*)));

    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)),
                     &app, SLOT(fileOpenRequested(QString)));

    int state = app.exec();

    return state;
}

