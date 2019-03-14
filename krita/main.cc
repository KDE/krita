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
#include <QStandardPaths>
#include <QDir>
#include <QDate>
#include <QLocale>
#include <QSettings>
#include <QByteArray>
#include <QMessageBox>
#include <QThread>

#if QT_VERSION >= 0x050900
#include <QOperatingSystemVersion>
#endif

#include <time.h>

#include <KisApplication.h>
#include <KoConfig.h>
#include <KoResourcePaths.h>
#include <kis_config.h>

#include "data/splash/splash_screen.xpm"
#include "data/splash/splash_holidays.xpm"
#include "data/splash/splash_screen_x2.xpm"
#include "data/splash/splash_holidays_x2.xpm"
#include "KisDocument.h"
#include "kis_splash_screen.h"
#include "KisPart.h"
#include "KisApplicationArguments.h"
#include <opengl/kis_opengl.h>
#include "input/KisQtWidgetsTweaker.h"
#include <KisUsageLogger.h>
#include <kis_image_config.h>

#if defined Q_OS_WIN
#include <windows.h>
#include <kis_tablet_support_win.h>
#include <kis_tablet_support_win8.h>
#include <QLibrary>
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

typedef enum ORIENTATION_PREFERENCE {
    ORIENTATION_PREFERENCE_NONE = 0x0,
    ORIENTATION_PREFERENCE_LANDSCAPE = 0x1,
    ORIENTATION_PREFERENCE_PORTRAIT = 0x2,
    ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED = 0x4,
    ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED = 0x8
} ORIENTATION_PREFERENCE;
typedef BOOL WINAPI (*pSetDisplayAutoRotationPreferences_t)(
        ORIENTATION_PREFERENCE orientation
        );
void resetRotation()
{
    QLibrary user32Lib("user32");
    if (!user32Lib.load()) {
        qWarning() << "Failed to load user32.dll! This really should not happen.";
        return;
    }
    pSetDisplayAutoRotationPreferences_t pSetDisplayAutoRotationPreferences
            = reinterpret_cast<pSetDisplayAutoRotationPreferences_t>(user32Lib.resolve("SetDisplayAutoRotationPreferences"));
    if (!pSetDisplayAutoRotationPreferences) {
        dbgKrita << "Failed to load function SetDisplayAutoRotationPreferences";
        return;
    }
    bool result = pSetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_NONE);
    dbgKrita << "SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_NONE) returned" << result;
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

    // Workaround a bug in QNetworkManager
    qputenv("QT_BEARER_POLL_TIMEOUT", QByteArray::number(-1));

    // A per-user unique string, without /, because QLocalServer cannot use names with a / in it
    QString key = "Krita4" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation).replace("/", "_");
    key = key.replace(":", "_").replace("\\","_");

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

#if QT_VERSION >= 0x050900
    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache, true);
#endif

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);

    bool singleApplication = true;
    bool enableOpenGLDebug = false;
    bool openGLDebugSynchronous = false;
    bool logUsage = true;
    {

        singleApplication = kritarc.value("EnableSingleApplication", true).toBool();
        if (kritarc.value("EnableHiDPI", true).toBool()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
        if (!qgetenv("KRITA_HIDPI").isEmpty()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }

        if (!qgetenv("KRITA_OPENGL_DEBUG").isEmpty()) {
            enableOpenGLDebug = true;
        } else {
            enableOpenGLDebug = kritarc.value("EnableOpenGLDebug", false).toBool();
        }
        if (enableOpenGLDebug && (qgetenv("KRITA_OPENGL_DEBUG") == "sync" || kritarc.value("OpenGLDebugSynchronous", false).toBool())) {
            openGLDebugSynchronous = true;
        }

        KisConfig::RootSurfaceFormat rootSurfaceFormat = KisConfig::rootSurfaceFormat(&kritarc);
        KisOpenGL::OpenGLRenderer preferredRenderer = KisOpenGL::RendererAuto;

        logUsage = kritarc.value("LogUsage", true).toBool();

        const QString preferredRendererString = kritarc.value("OpenGLRenderer", "auto").toString();
        preferredRenderer = KisOpenGL::convertConfigToOpenGLRenderer(preferredRendererString);

#ifdef Q_OS_WIN
        // Force ANGLE to use Direct3D11. D3D9 doesn't support OpenGL ES 3 and WARP
        //  might get weird crashes atm.
        qputenv("QT_ANGLE_PLATFORM", "d3d11");
#endif

        const QSurfaceFormat format =
            KisOpenGL::selectSurfaceFormat(preferredRenderer, rootSurfaceFormat, enableOpenGLDebug);

        if (format.renderableType() == QSurfaceFormat::OpenGLES) {
            QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
        } else {
            QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
        }
        KisOpenGL::setDefaultSurfaceFormat(format);
        KisOpenGL::setDebugSynchronous(openGLDebugSynchronous);

#ifdef Q_OS_WIN
        // HACK: https://bugs.kde.org/show_bug.cgi?id=390651
        resetRotation();
#endif
    }

    if (logUsage) {
        KisUsageLogger::initialize();
    }


    QString root;
    QString language;
    {
        // Create a temporary application to get the root
        QCoreApplication app(argc, argv);
        Q_UNUSED(app);
        root = KoResourcePaths::getApplicationRoot();
        QSettings languageoverride(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat);
        languageoverride.beginGroup(QStringLiteral("Language"));
        language = languageoverride.value(qAppName(), "").toString();
    }


#ifdef Q_OS_LINUX
    {
        QByteArray originalXdgDataDirs = qgetenv("XDG_DATA_DIRS");
        if (originalXdgDataDirs.isEmpty()) {
            // We don't want to completely override the default
            originalXdgDataDirs = "/usr/local/share/:/usr/share/";
        }
        qputenv("XDG_DATA_DIRS", QFile::encodeName(root + "share") + ":" + originalXdgDataDirs);
    }
#else
    qputenv("XDG_DATA_DIRS", QFile::encodeName(root + "share"));
#endif

    dbgKrita << "Setting XDG_DATA_DIRS" << qgetenv("XDG_DATA_DIRS");

    // Now that the paths are set, set the language. First check the override from the language
    // selection dialog.

    dbgKrita << "Override language:" << language;
    bool rightToLeft = false;
    if (!language.isEmpty()) {
        KLocalizedString::setLanguages(language.split(":"));
        // And override Qt's locale, too
        qputenv("LANG", language.split(":").first().toLocal8Bit());
        QLocale locale(language.split(":").first());
        QLocale::setDefault(locale);

        const QStringList rtlLanguages = QStringList()
                << "ar" << "dv" << "he" << "ha" << "ku" << "fa" << "ps" << "ur" << "yi";

        if (rtlLanguages.contains(language.split(':').first())) {
            rightToLeft = true;
        }
    }
    else {
        dbgKrita << "Qt UI languages:" << QLocale::system().uiLanguages() << qgetenv("LANG");

        // And if there isn't one, check the one set by the system.
        QLocale locale = QLocale::system();
        if (locale.name() != QStringLiteral("en")) {
            QStringList uiLanguages = locale.uiLanguages();
            for (QString &uiLanguage : uiLanguages) {

                // This list of language codes that can have a specifier should
                // be extended whenever we have translations that need it; right
                // now, only en, pt, zh are in this situation.

                if (uiLanguage.startsWith("en") || uiLanguage.startsWith("pt")) {
                    uiLanguage.replace(QChar('-'), QChar('_'));
                }
                else if (uiLanguage.startsWith("zh-Hant") || uiLanguage.startsWith("zh-TW")) {
                    uiLanguage = "zh_TW";
                }
                else if (uiLanguage.startsWith("zh-Hans") || uiLanguage.startsWith("zh-CN")) {
                    uiLanguage = "zh_CN";
                }
            }

            for (int i = 0; i < uiLanguages.size(); i++) {
                QString uiLanguage = uiLanguages[i];
                // Strip the country code
                int idx = uiLanguage.indexOf(QChar('-'));

                if (idx != -1) {
                    uiLanguage = uiLanguage.left(idx);
                    uiLanguages.replace(i, uiLanguage);
                }
            }
            dbgKrita << "Converted ui languages:" << uiLanguages;
            qputenv("LANG", uiLanguages.first().toLocal8Bit());
#ifdef Q_OS_MAC
            // See https://bugs.kde.org/show_bug.cgi?id=396370
            KLocalizedString::setLanguages(QStringList() << uiLanguages.first());
#else
            KLocalizedString::setLanguages(QStringList() << uiLanguages);
#endif
        }
    }

    // first create the application so we can create a pixmap
    KisApplication app(key, argc, argv);

    KisUsageLogger::writeHeader();

    if (!language.isEmpty()) {
        if (rightToLeft) {
            app.setLayoutDirection(Qt::RightToLeft);
        }
        else {
            app.setLayoutDirection(Qt::LeftToRight);
        }
    }
    KLocalizedString::setApplicationDomain("krita");

    dbgKrita << "Available translations" << KLocalizedString::availableApplicationTranslations();
    dbgKrita << "Available domain translations" << KLocalizedString::availableDomainTranslations("krita");


#ifdef Q_OS_WIN
    QDir appdir(KoResourcePaths::getApplicationRoot());
    QString path = qgetenv("PATH");
    qputenv("PATH", QFile::encodeName(appdir.absolutePath() + "/bin" + ";"
                                      + appdir.absolutePath() + "/lib" + ";"
                                      + appdir.absolutePath() + "/Frameworks" + ";"
                                      + appdir.absolutePath() + ";"
                                      + path));

    dbgKrita << "PATH" << qgetenv("PATH");
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
        const bool batchRun = args.exportAs();

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

    app.installEventFilter(KisQtWidgetsTweaker::instance());

    if (!args.noSplash()) {
        // then create the pixmap from an xpm: we cannot get the
        // location of our datadir before we've started our components,
        // so use an xpm.
        QDate currentDate = QDate::currentDate();
        QWidget *splash = 0;
        if (currentDate > QDate(currentDate.year(), 12, 4) ||
                currentDate < QDate(currentDate.year(), 1, 9)) {
            splash = new KisSplashScreen(app.applicationVersion(), QPixmap(splash_holidays_xpm), QPixmap(splash_holidays_x2_xpm));
        }
        else {
            splash = new KisSplashScreen(app.applicationVersion(), QPixmap(splash_screen_xpm), QPixmap(splash_screen_x2_xpm));
        }

        app.setSplashScreen(splash);
    }

#if defined Q_OS_WIN
    KisConfig cfg(false);
    bool supportedWindowsVersion = true;
#if QT_VERSION >= 0x050900
    QOperatingSystemVersion osVersion = QOperatingSystemVersion::current();
    if (osVersion.type() == QOperatingSystemVersion::Windows) {
        if (osVersion.majorVersion() >= QOperatingSystemVersion::Windows7.majorVersion()) {
            supportedWindowsVersion  = true;
        }
        else {
            supportedWindowsVersion  = false;
            if (cfg.readEntry("WarnedAboutUnsupportedWindows", false)) {
                QMessageBox::information(0,
                                         i18nc("@title:window", "Krita: Warning"),
                                         i18n("You are running an unsupported version of Windows: %1.\n"
                                              "This is not recommended. Do not report any bugs.\n"
                                              "Please update to a supported version of Windows: Windows 7, 8, 8.1 or 10.", osVersion.name()));
                cfg.writeEntry("WarnedAboutUnsupportedWindows", true);

            }
        }
    }
#endif

    {
        if (cfg.useWin8PointerInput() && !KisTabletSupportWin8::isAvailable()) {
            cfg.setUseWin8PointerInput(false);
        }
        if (!cfg.useWin8PointerInput()) {
            bool hasWinTab = KisTabletSupportWin::init();
            if (!hasWinTab && supportedWindowsVersion) {
                if (KisTabletSupportWin8::isPenDeviceAvailable()) {
                    // Use WinInk automatically
                    cfg.setUseWin8PointerInput(true);
                } else if (!cfg.readEntry("WarnedAboutMissingWinTab", false)) {
                    if (KisTabletSupportWin8::isAvailable()) {
                        QMessageBox::information(nullptr,
                                                 i18n("Krita Tablet Support"),
                                                 i18n("Cannot load WinTab driver and no Windows Ink pen devices are found. If you have a drawing tablet, please make sure the tablet driver is properly installed."),
                                                 QMessageBox::Ok, QMessageBox::Ok);
                    } else {
                        QMessageBox::information(nullptr,
                                                 i18n("Krita Tablet Support"),
                                                 i18n("Cannot load WinTab driver. If you have a drawing tablet, please make sure the tablet driver is properly installed."),
                                                 QMessageBox::Ok, QMessageBox::Ok);
                    }
                    cfg.writeEntry("WarnedAboutMissingWinTab", true);
                }
            }
        }
        if (cfg.useWin8PointerInput()) {
            KisTabletSupportWin8 *penFilter = new KisTabletSupportWin8();
            if (penFilter->init()) {
                // penFilter.registerPointerDeviceNotifications();
                app.installNativeEventFilter(penFilter);
                dbgKrita << "Using Win8 Pointer Input for tablet support";
            } else {
                dbgKrita << "No Win8 Pointer Input available";
                delete penFilter;
            }
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

    // Hardware information
    KisUsageLogger::write("\nHardware Information\n");
    KisUsageLogger::write(QString("  GPU Acceleration: %1").arg(kritarc.value("OpenGLRenderer", "auto").toString()));
    KisUsageLogger::write(QString("  Memory: %1 Mb").arg(KisImageConfig(true).totalRAM()));
    KisUsageLogger::write(QString("  Number of Cores: %1").arg(QThread::idealThreadCount()));
    KisUsageLogger::write(QString("  Swap Location: %1\n").arg(KisImageConfig(true).swapDir()));

    int state = app.exec();

    {
        QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
        kritarc.setValue("canvasState", "OPENGL_SUCCESS");
    }

    if (logUsage) {
        KisUsageLogger::close();
    }

    return state;
}
