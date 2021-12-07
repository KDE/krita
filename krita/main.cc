/*
* SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
* SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
* SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <QLibraryInfo>
#include <QTranslator>

#include <QOperatingSystemVersion>

#include <time.h>

#include <KisApplication.h>
#include <KoConfig.h>
#include <KoResourcePaths.h>
#include <kis_config.h>

#include "KisDocument.h"
#include "kis_splash_screen.h"
#include "KisPart.h"
#include "KisApplicationArguments.h"
#include <opengl/kis_opengl.h>
#include "input/KisQtWidgetsTweaker.h"
#include <KisUsageLogger.h>
#include <kis_image_config.h>
#include "KisUiFont.h"

#include <KLocalizedTranslator>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

#if defined Q_OS_WIN
#include "config_use_qt_tablet_windows.h"
#include <windows.h>
#ifndef USE_QT_TABLET_WINDOWS
#include <kis_tablet_support_win.h>
#include <kis_tablet_support_win8.h>
#else
#include <dialogs/KisDlgCustomTabletResolution.h>
#endif
#include "config-high-dpi-scale-factor-rounding-policy.h"
#include "config-set-has-border-in-full-screen-default.h"
#ifdef HAVE_SET_HAS_BORDER_IN_FULL_SCREEN_DEFAULT
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif
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
} // namespace
#endif

namespace
{

void installTranslators(KisApplication &app);

} // namespace

#ifdef Q_OS_WIN
namespace
{
typedef enum ORIENTATION_PREFERENCE {
    ORIENTATION_PREFERENCE_NONE = 0x0,
    ORIENTATION_PREFERENCE_LANDSCAPE = 0x1,
    ORIENTATION_PREFERENCE_PORTRAIT = 0x2,
    ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED = 0x4,
    ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED = 0x8
} ORIENTATION_PREFERENCE;
#if !defined(_MSC_VER)
    typedef BOOL WINAPI (*pSetDisplayAutoRotationPreferences_t)(
            ORIENTATION_PREFERENCE orientation
            );
#else
    typedef BOOL (WINAPI *pSetDisplayAutoRotationPreferences_t)(
        ORIENTATION_PREFERENCE orientation
        );
#endif()
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

#ifdef Q_OS_ANDROID
extern "C" JNIEXPORT void JNICALL
Java_org_krita_android_JNIWrappers_saveState(JNIEnv* /*env*/,
                                             jobject /*obj*/,
                                             jint    /*n*/)
{
    if (!KisPart::exists()) return;

    KisPart *kisPart = KisPart::instance();
    QList<QPointer<KisDocument>> list = kisPart->documents();
    for (QPointer<KisDocument> &doc: list)
    {
        doc->autoSaveOnPause();
    }

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
    kritarc.setValue("canvasState", "OPENGL_SUCCESS");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_krita_android_JNIWrappers_exitFullScreen(JNIEnv* /*env*/,
                                                  jobject /*obj*/,
                                                  jint    /*n*/)
{
    if (!KisPart::exists()) {
        return false;
    }

    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    if (mainWindow && mainWindow->isFullScreen()) {
        // since, this calls KisConfig, we need to make sure it happens on that
        // thread (we get here from the Android Main thread)
        QMetaObject::invokeMethod(mainWindow, "viewFullscreen",
                                  Qt::QueuedConnection, Q_ARG(bool, false));
        return true;
    } else {
        return false;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_krita_android_JNIWrappers_hasMainWindowLoaded(JNIEnv * /*env*/,
                                                       jobject /*obj*/,
                                                       jint /*n*/)
{
    if (!KisPart::exists()) {
        return false;
    }

    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    return (bool)mainWindow;
}

extern "C" JNIEXPORT void JNICALL
Java_org_krita_android_JNIWrappers_openFileFromIntent(JNIEnv* /*env*/,
                                                      jobject /*obj*/,
                                                      jstring str)
{
    QAndroidJniObject jUri(str);
    if (jUri.isValid()) {
        QString uri = jUri.toString();
        QMetaObject::invokeMethod(KisApplication::instance(), "fileOpenRequested",
                                  Qt::QueuedConnection, Q_ARG(QString, uri));
    }
}

#define MAIN_EXPORT __attribute__ ((visibility ("default")))
#define MAIN_FN main
#elif defined Q_OS_WIN
#define MAIN_EXPORT __declspec(dllexport)
#define MAIN_FN krita_main
#else
#define MAIN_EXPORT
#define MAIN_FN main
#endif

extern "C" MAIN_EXPORT int MAIN_FN(int argc, char **argv)
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
    QString key = "Krita5" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation).replace("/", "_");
    key = key.replace(":", "_").replace("\\","_");

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache, true);

#ifdef HAVE_HIGH_DPI_SCALE_FACTOR_ROUNDING_POLICY
    // This rounding policy depends on a series of patches to Qt related to
    // https://bugreports.qt.io/browse/QTBUG-53022. These patches are applied
    // in ext_qt for WIndows (patches 0031-0036).
    //
    // The rounding policy can be set externally by setting the environment
    // variable `QT_SCALE_FACTOR_ROUNDING_POLICY` to one of the following:
    //   Round:            Round up for .5 and above.
    //   Ceil:             Always round up.
    //   Floor:            Always round down.
    //   RoundPreferFloor: Round up for .75 and above.
    //   PassThrough:      Don't round.
    //
    // The default is set to RoundPreferFloor for better behaviour than before,
    // but can be overridden by the above environment variable.
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#endif

#ifdef Q_OS_ANDROID
    const QString write_permission = "android.permission.WRITE_EXTERNAL_STORAGE";
    const QStringList permissions = { write_permission };
    const QtAndroid::PermissionResultMap resultHash =
            QtAndroid::requestPermissionsSync(QStringList(permissions));

    if (resultHash[write_permission] == QtAndroid::PermissionResult::Denied) {
        // TODO: show a dialog and graciously exit
        dbgKrita << "Permission denied by the user";
    }
    else {
        dbgKrita << "Permission granted";
    }
#endif

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);

    bool enableOpenGLDebug = false;
    bool openGLDebugSynchronous = false;
    bool logUsage = true;
    {
        if (kritarc.value("EnableHiDPI", true).toBool()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
        if (!qgetenv("KRITA_HIDPI").isEmpty()) {
            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
#ifdef HAVE_HIGH_DPI_SCALE_FACTOR_ROUNDING_POLICY
        if (kritarc.value("EnableHiDPIFractionalScaling", false).toBool()) {
            QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
        }
#endif

        if (!qEnvironmentVariableIsEmpty("KRITA_OPENGL_DEBUG")) {
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

#ifdef Q_OS_WIN
        const QString preferredRendererString = kritarc.value("OpenGLRenderer", "angle").toString();
#else
        const QString preferredRendererString = kritarc.value("OpenGLRenderer", "auto").toString();
#endif
        preferredRenderer = KisOpenGL::convertConfigToOpenGLRenderer(preferredRendererString);

        const KisOpenGL::RendererConfig config =
            KisOpenGL::selectSurfaceConfig(preferredRenderer, rootSurfaceFormat, enableOpenGLDebug);

        KisOpenGL::setDefaultSurfaceConfig(config);
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

        // APPIMAGE SOUND ADDITIONS
        // GStreamer needs a few environment variables to properly function in an appimage context.
        // The following code should be configured to **only** run when we detect that Krita is being
        // run within an appimage. Checking for the presence of an APPDIR path env variable seems to be
        // enough to filter out this step for non-appimage krita builds.

        const bool isInAppimage = qEnvironmentVariableIsSet("APPIMAGE");
        if (isInAppimage) {
            QByteArray appimageMountDir = qgetenv("APPDIR");

            //We need to add new gstreamer plugin paths for the system to find the
            //appropriate plugins.
            const QByteArray gstPluginSystemPath = qgetenv("GST_PLUGIN_SYSTEM_PATH_1_0");
            const QByteArray gstPluginScannerPath = qgetenv("GST_PLUGIN_SCANNER");

            //Plugins Path is where libgstreamer-1.0 should expect to find plugin libraries.
            qputenv("GST_PLUGIN_SYSTEM_PATH_1_0", appimageMountDir + QFile::encodeName("/usr/lib/gstreamer-1.0/") + ":" + gstPluginSystemPath);

            //Plugin scanner is where gstreamer should expect to find the plugin scanner.
            //Perhaps invoking the scanenr earlier in the code manually could allow ldd to quickly find all plugin dependencies?
            qputenv("GST_PLUGIN_SCANNER", appimageMountDir + QFile::encodeName("/usr/lib/gstreamer-1.0/gst-plugin-scanner"));
        }
    }
#else
    qputenv("XDG_DATA_DIRS", QFile::encodeName(root + "share"));
#endif

    dbgKrita << "Setting XDG_DATA_DIRS" << qgetenv("XDG_DATA_DIRS");

    // Now that the paths are set, set the language. First check the override from the language
    // selection dialog.

    dbgLocale << "Override language:" << language;
    bool rightToLeft = false;
    if (!language.isEmpty()) {
        KLocalizedString::setLanguages(language.split(":"));

        // And override Qt's locale, too
        QLocale locale(language.split(":").first());
        QLocale::setDefault(locale);
#ifdef Q_OS_MAC
        // prevents python >=3.7 nl_langinfo(CODESET) fail bug 417312.
        qputenv("LANG", (locale.name() + ".UTF-8").toLocal8Bit());
#else
        qputenv("LANG", locale.name().toLocal8Bit());
#endif

        const QStringList rtlLanguages = QStringList()
                << "ar" << "dv" << "he" << "ha" << "ku" << "fa" << "ps" << "ur" << "yi";

        if (rtlLanguages.contains(language.split(':').first())) {
            rightToLeft = true;
        }
    }
    else {
        dbgLocale << "Qt UI languages:" << QLocale::system().uiLanguages() << qgetenv("LANG");

        // And if there isn't one, check the one set by the system.
        QLocale locale = QLocale::system();

#ifdef Q_OS_ANDROID
        // QLocale::uiLanguages() fails on Android, so if the fallback locale is being
        // used we, try to fetch the device's default locale.
        if (locale.name() == QLocale::c().name()) {
            QAndroidJniObject localeJniObj = QAndroidJniObject::callStaticObjectMethod(
                "java/util/Locale", "getDefault", "()Ljava/util/Locale;");

            if (localeJniObj.isValid()) {
                QAndroidJniObject tag = localeJniObj.callObjectMethod("toLanguageTag",
                                                                      "()Ljava/lang/String;");
                if (tag.isValid()) {
                    locale = QLocale(tag.toString());
                }
            }
        }
#endif
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

            if (uiLanguages.size() > 0 ) {
                QString envLanguage = uiLanguages.first();
                envLanguage.replace(QChar('-'), QChar('_'));

                for (int i = 0; i < uiLanguages.size(); i++) {
                    QString uiLanguage = uiLanguages[i];
                    // Strip the country code
                    int idx = uiLanguage.indexOf(QChar('-'));

                    if (idx != -1) {
                        uiLanguage = uiLanguage.left(idx);
                        uiLanguages.replace(i, uiLanguage);
                    }
                }
                dbgLocale << "Converted ui languages:" << uiLanguages;
#ifdef Q_OS_MAC
                // See https://bugs.kde.org/show_bug.cgi?id=396370
                KLocalizedString::setLanguages(QStringList() << uiLanguages.first());
                qputenv("LANG", (envLanguage + ".UTF-8").toLocal8Bit());
#else
                KLocalizedString::setLanguages(QStringList() << uiLanguages);
                qputenv("LANG", envLanguage.toLocal8Bit());
#endif
            }
        }
    }

#if defined Q_OS_WIN && defined USE_QT_TABLET_WINDOWS && defined QT_HAS_WINTAB_SWITCH
    const bool forceWinTab = !KisConfig::useWin8PointerInputNoApp(&kritarc);
    QCoreApplication::setAttribute(Qt::AA_MSWindowsUseWinTabAPI, forceWinTab);

    if (qEnvironmentVariableIsEmpty("QT_WINTAB_DESKTOP_RECT") &&
        qEnvironmentVariableIsEmpty("QT_IGNORE_WINTAB_MAPPING")) {

        QRect customTabletRect;
        KisDlgCustomTabletResolution::Mode tabletMode =
            KisDlgCustomTabletResolution::getTabletMode(&customTabletRect);
        KisDlgCustomTabletResolution::applyConfiguration(tabletMode, customTabletRect);
    }
#endif

    // first create the application so we can create a pixmap
    KisApplication app(key, argc, argv);

    installTranslators(app);

    if (app.platformName() == "wayland") {
        QMessageBox::critical(0, i18nc("@title:window", "Fatal Error"), i18n("Krita does not support the Wayland platform. Use XWayland to run Krita on Wayland. Krita will close now."));
        return -1;
    }

    KisUsageLogger::writeHeader();
    KisOpenGL::initialize();

#ifdef HAVE_SET_HAS_BORDER_IN_FULL_SCREEN_DEFAULT
    if (QCoreApplication::testAttribute(Qt::AA_UseDesktopOpenGL)) {
        QWindowsWindowFunctions::setHasBorderInFullScreenDefault(true);
    }
#endif


    if (!language.isEmpty()) {
        if (rightToLeft) {
            app.setLayoutDirection(Qt::RightToLeft);
        }
        else {
            app.setLayoutDirection(Qt::LeftToRight);
        }
    }
#ifdef Q_OS_ANDROID
    KisApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

    // TODO: remove "share" - sh_zam
    // points to /data/data/org.krita/files/share/locale
    KLocalizedString::addDomainLocaleDir("krita", QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/share/locale");
#endif

    KLocalizedString::setApplicationDomain("krita");

    dbgLocale << "Available translations" << KLocalizedString::availableApplicationTranslations();
    dbgLocale << "Available domain translations" << KLocalizedString::availableDomainTranslations("krita");


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

    KisApplicationArguments args(app);

    if (app.isRunning()) {
        // only pass arguments to main instance if they are not for batch processing
        // any batch processing would be done in this separate instance
        const bool batchRun = args.exportAs() || args.exportSequence();

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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif
    app.installEventFilter(KisQtWidgetsTweaker::instance());

    if (!args.noSplash()) {
        QWidget *splash = new KisSplashScreen();
        app.setSplashScreen(splash);
    }

#if defined Q_OS_WIN
    KisConfig cfg(false);
    bool supportedWindowsVersion = true;
    QOperatingSystemVersion osVersion = QOperatingSystemVersion::current();
    if (osVersion.type() == QOperatingSystemVersion::Windows) {
        if (osVersion.majorVersion() >= QOperatingSystemVersion::Windows7.majorVersion()) {
            supportedWindowsVersion  = true;
        }
        else {
            supportedWindowsVersion  = false;
            if (cfg.readEntry("WarnedAboutUnsupportedWindows", false)) {
                QMessageBox::information(nullptr,
                                         i18nc("@title:window", "Krita: Warning"),
                                         i18n("You are running an unsupported version of Windows: %1.\n"
                                              "This is not recommended. Do not report any bugs.\n"
                                              "Please update to a supported version of Windows: Windows 7, 8, 8.1 or 10.", osVersion.name()));
                cfg.writeEntry("WarnedAboutUnsupportedWindows", true);

            }
        }
    }
#ifndef USE_QT_TABLET_WINDOWS
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
#elif defined QT_HAS_WINTAB_SWITCH

    // Check if WinTab/WinInk has actually activated
    const bool useWinInkAPI = !app.testAttribute(Qt::AA_MSWindowsUseWinTabAPI);

    if (useWinInkAPI != cfg.useWin8PointerInput()) {
        KisUsageLogger::log("WARNING: WinTab tablet protocol is not supported on this device. Switching to WinInk...");

        cfg.setUseWin8PointerInput(useWinInkAPI);
        cfg.setUseRightMiddleTabletButtonWorkaround(true);
    }

#endif
#endif
    app.setAttribute(Qt::AA_CompressHighFrequencyEvents, false);

    // Set up remote arguments.
    QObject::connect(&app, SIGNAL(messageReceived(QByteArray,QObject*)),
                     &app, SLOT(remoteArguments(QByteArray,QObject*)));

    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)),
                     &app, SLOT(fileOpenRequested(QString)));

    // Hardware information
    KisUsageLogger::writeSysInfo("\nHardware Information\n");
    KisUsageLogger::writeSysInfo(QString("  GPU Acceleration: %1").arg(kritarc.value("OpenGLRenderer", "auto").toString()));
    KisUsageLogger::writeSysInfo(QString("  Memory: %1 Mb").arg(KisImageConfig(true).totalRAM()));
    KisUsageLogger::writeSysInfo(QString("  Number of Cores: %1").arg(QThread::idealThreadCount()));
    KisUsageLogger::writeSysInfo(QString("  Swap Location: %1\n").arg(KisImageConfig(true).swapDir()));

    KisConfig(true).logImportantSettings();

    app.setFont(KisUiFont::normalFont());

    if (!app.start(args)) {
        KisUsageLogger::log("Could not start Krita Application");
        return 1;
    }


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

namespace
{

void removeInstalledTranslators(KisApplication &app)
{
    // HACK: We try to remove all the translators installed by ECMQmLoader.
    // The reason is that it always load translations for the system locale
    // which interferes with our effort to handle override languages. Since
    // `en_US` (or `en`) strings are defined in code, the QTranslator doesn't
    // actually handle translations for them, so even if we try to install
    // a QTranslator loaded from `en`, the strings always get translated by
    // the system language QTranslator that ECMQmLoader installed instead
    // of the English one.

    // ECMQmLoader creates all QTranslator's parented to the active QApp.
    QList<QTranslator *> translators = app.findChildren<QTranslator *>(QString(), Qt::FindDirectChildrenOnly);
    Q_FOREACH(const auto &translator, translators) {
        app.removeTranslator(translator);
    }
    dbgLocale << "Removed" << translators.size() << "QTranslator's";
}

void installPythonPluginUITranslator(KisApplication &app)
{
    // Install a KLocalizedTranslator, so that when the bundled Python plugins
    // load their UI files using uic.loadUi() it can be translated.
    // These UI files must specify "pykrita_plugin_ui" as their class names.
    KLocalizedTranslator *translator = new KLocalizedTranslator(&app);
    translator->setObjectName(QStringLiteral("KLocalizedTranslator.pykrita_plugin_ui"));
    translator->setTranslationDomain(QStringLiteral("krita"));
    translator->addContextToMonitor(QStringLiteral("pykrita_plugin_ui"));
    app.installTranslator(translator);
}

void installQtTranslations(KisApplication &app)
{
    QStringList qtCatalogs = {
        QStringLiteral("qt_"),
        QStringLiteral("qtbase_"),
        QStringLiteral("qtmultimedia_"),
        QStringLiteral("qtdeclarative_"),
    };
    // A list of locale to add, note that the last added one has the
    // highest precedence.
    QList<QLocale> localeList;
    // We always use English as the final fallback.
    localeList.append(QLocale(QLocale::English));
    QLocale defaultLocale;
    if (defaultLocale.language() != QLocale::English) {
        localeList.append(defaultLocale);
    }

    QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    dbgLocale << "Qt translations path:" << translationsPath;

    Q_FOREACH(const auto &localeToLoad, localeList) {
        Q_FOREACH(const auto &catalog, qtCatalogs) {
            QTranslator *translator = new QTranslator(&app);
            if (translator->load(localeToLoad, catalog, QString(), translationsPath)) {
                dbgLocale << "Loaded Qt translations for" << localeToLoad << catalog;
                translator->setObjectName(QStringLiteral("QTranslator.%1.%2").arg(localeToLoad.name(), catalog));
                app.installTranslator(translator);
            } else {
                delete translator;
            }
        }
    }
}

void installEcmTranslations(KisApplication &app)
{
    // Load translations created using the ECMPoQmTools module.
    // This function is based on the code in:
    // https://invent.kde.org/frameworks/extra-cmake-modules/-/blob/master/modules/ECMQmLoader.cpp.in

    QStringList ecmCatalogs = {
        QStringLiteral("kcompletion5_qt"),
        QStringLiteral("kconfig5_qt"),
        QStringLiteral("kcoreaddons5_qt"),
        QStringLiteral("kitemviews5_qt"),
        QStringLiteral("kwidgetsaddons5_qt"),
        QStringLiteral("kwindowsystem5_qt"),
        QStringLiteral("seexpr2_qt"),
    };

    QStringList ki18nLangs = KLocalizedString::languages();
    const QString langEn = QStringLiteral("en");
    // Replace "en_US" with "en" because that's what we have in the locale dir.
    int indexOfEnUs = ki18nLangs.indexOf(QStringLiteral("en_US"));
    if (indexOfEnUs != -1) {
        ki18nLangs[indexOfEnUs] = langEn;
    }
    // We need to have "en" to the end of the list, because we explicitly
    // removed the "en" translators added by ECMQmLoader.
    // If "en" is already on the list, we truncate the ones after, because
    // "en" is the catch-all fallback that has the strings in code.
    int indexOfEn = ki18nLangs.indexOf(langEn);
    if (indexOfEn != -1) {
        for (int i = ki18nLangs.size() - indexOfEn - 1; i > 0; i--) {
            ki18nLangs.removeLast();
        }
    } else {
        ki18nLangs.append(langEn);
    }

    // The last added one has the highest precedence, so we iterate the
    // list backwards.
    QStringListIterator langIter(ki18nLangs);
    langIter.toBack();

    while (langIter.hasPrevious()) {
        const QString &localeDirName = langIter.previous();
        Q_FOREACH(const auto &catalog, ecmCatalogs) {
            QString subPath = QStringLiteral("locale/") % localeDirName % QStringLiteral("/LC_MESSAGES/") % catalog % QStringLiteral(".qm");
#if defined(Q_OS_ANDROID)
            const QString fullPath = QStringLiteral("assets:/share/") + subPath;
            if (!QFile::exists(fullPath)) {
                continue;
            }
#else
            // Our patched k18n uses AppDataLocation (for AppImage).
            QString fullPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, subPath);
            if (fullPath.isEmpty()) {
                // ... but distro builds probably still use GenericDataLocation,
                // so check that too.
                fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, subPath);
                if (fullPath.isEmpty()) {
                    continue;
                }
            }
#endif
            QTranslator *translator = new QTranslator(&app);
            if (translator->load(fullPath)) {
                dbgLocale << "Loaded ECM translations for" << localeDirName << catalog;
                translator->setObjectName(QStringLiteral("QTranslator.%1.%2").arg(localeDirName, catalog));
                app.installTranslator(translator);
            } else {
                delete translator;
            }
        }
    }
}

void installTranslators(KisApplication &app)
{
    removeInstalledTranslators(app);
    installPythonPluginUITranslator(app);
    installQtTranslations(app);
    installEcmTranslations(app);
}

} // namespace
