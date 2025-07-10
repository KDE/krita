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

#include <KLocalizedTranslator>
#include <QByteArray>
#include <QDate>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QOperatingSystemVersion>
#include <QPixmap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QThread>
#include <QTranslator>
#include <QImageReader>

#include <KisApplication.h>
#include <KisMainWindow.h>
#include <KisSupportedArchitectures.h>
#include <KisUsageLogger.h>
#include <KoConfig.h>
#include <KoResourcePaths.h>
#include <kis_config.h>
#include <kis_debug.h>
#include <kis_image_config.h>
#include <opengl/kis_opengl.h>

#include "KisApplicationArguments.h"
#include "KisDocument.h"
#include "KisPart.h"
#include "KisUiFont.h"
#include "input/KisQtWidgetsTweaker.h"
#include "kis_splash_screen.h"
#include "config-qt-patches-present.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <KisAndroidCrashHandler.h>
#include <KisAndroidLogHandler.h>
#endif

#if defined Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
// this include is Qt5-only, the switch to WinTab is embedded in Qt
#  include "config_qt5_has_wintab_switch.h"
#else
#  include <QtGui/private/qguiapplication_p.h>
#  include <QtGui/qpa/qplatformintegration.h>
#endif
#include <windows.h>
#include <winuser.h>
#include <dialogs/KisDlgCustomTabletResolution.h>
#include "config-high-dpi-scale-factor-rounding-policy.h"
#include "config-set-has-border-in-full-screen-default.h"
#ifdef HAVE_SET_HAS_BORDER_IN_FULL_SCREEN_DEFAULT
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif
#include <QLibrary>
#endif

#ifdef Q_OS_MACOS
#include "libs/macosutils/KisMacosEntitlements.h"
#include "libs/macosutils/KisMacosSystemProber.h"
#endif

#ifdef Q_OS_HAIKU
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif

#if defined HAVE_KCRASH
#include <kcrash.h>
#elif defined USE_DRMINGW
namespace
{
template<typename T, typename U>
inline T cast_to_function(U v) noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<T>(reinterpret_cast<void *>(v));
}

void tryInitDrMingw()
{
    const QString pathStr = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("exchndl.dll");

    QLibrary hMod(pathStr);
    if (!hMod.load()) {
        return;
    }

    using ExcHndlSetLogFileNameA_type = BOOL(APIENTRY *)(const char *);

    // No need to call ExcHndlInit since the crash handler is installed on DllMain
    const auto myExcHndlSetLogFileNameA = cast_to_function<ExcHndlSetLogFileNameA_type>(hMod.resolve("ExcHndlSetLogFileNameA"));
    if (!myExcHndlSetLogFileNameA) {
        return;
    }

    // Set the log file path to %LocalAppData%\kritacrash.log
    const QString logFile = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).absoluteFilePath("kritacrash.log");
    const QByteArray logFilePath = QDir::toNativeSeparators(logFile).toLocal8Bit();
    myExcHndlSetLogFileNameA(logFilePath.data());
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
using pSetDisplayAutoRotationPreferences_t = decltype(&SetDisplayAutoRotationPreferences);

void resetRotation()
{
    QLibrary user32Lib("user32");
    if (!user32Lib.load()) {
        qWarning() << "Failed to load user32.dll! This really should not happen.";
        return;
    }
    auto pSetDisplayAutoRotationPreferences = cast_to_function<pSetDisplayAutoRotationPreferences_t>(user32Lib.resolve("SetDisplayAutoRotationPreferences"));
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
#ifdef Q_OS_ANDROID
    KisAndroidLogHandler::handler_init();
#endif

#ifdef Q_OS_WIN
    // Fix QCommandLineParser help output with UTF-8 codepage:
    if (GetACP() == CP_UTF8) {
        SetConsoleOutputCP(CP_UTF8);
    }
#endif

    bool runningInKDE = !qgetenv("KDE_FULL_SESSION").isEmpty();

#if defined HAVE_X11
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#elif defined Q_OS_WIN
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "windows:darkmode=1");
    }
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

    // In Qt6, QImageReader has an allocation limit to prevent large memory allocations.
    // However in Qt5 this doesn't exist, and can easily trigger in KisFileIconCreator while creating icons on large thumbnails.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(0);
#endif

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

    KisAndroidCrashHandler::handler_init();
    qputenv("QT_ANDROID_ENABLE_RIGHT_MOUSE_FROM_LONG_PRESS", "1");

    qputenv("FONTCONFIG_PATH",
            QFile::encodeName(KoResourcePaths::getApplicationRoot()) + "/share/etc/fonts/");
    qputenv("XDG_CACHE_HOME",
            QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
#endif

/**
 * MLT installation notes.
 *
 * On Linux and Android MLT is installed into a versioned
 * location, i.e. 'mlt-7'. On Windows and MacOS into a non-versioned
 * one, i.e. 'mlt'.
 *
 * On Windows and MacOS MLT uses detects all the paths via the relative
 * path against the currently running executable, so we don't have to
 * configure anything special for these platforms.
 *
 * On Linux and Android MLT does not have such detection, so we should
 * configure environment variables manually and instruct MLT where to
 * look for plugins, profiles and presets. Otherwise MLT will look for
 * plugins from the **build environment** location.
 */
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    // APPIMAGE SOUND ADDITIONS
    // MLT needs a few environment variables set to properly function in an appimage context.
    // The following code should be configured to **only** run when we detect that Krita is being
    // run within an appimage. Checking for the presence of an APPDIR path env variable seems to be
    // enough to filter out this step for non-appimage krita builds.
    const bool isInAppimage = qEnvironmentVariableIsSet("APPIMAGE");
    if (isInAppimage) {
        QString appimageMountDir = qgetenv("APPDIR");

        {   // MLT
            //Plugins Path is where mlt should expect to find its plugin libraries.
            const QString mltLibs = "/usr/lib/mlt-7";
            const QString mltData = "/usr/share/mlt-7";

            qputenv("MLT_ROOT_DIR", (appimageMountDir + "/usr").toUtf8());
            qputenv("MLT_REPOSITORY", (appimageMountDir + mltLibs).toUtf8());
            qputenv("MLT_DATA", (appimageMountDir + mltData).toUtf8());
            qputenv("MLT_PROFILES_PATH", (appimageMountDir + mltData + "/profiles").toUtf8());
            qputenv("MLT_PRESETS_PATH", (appimageMountDir + mltData + "/presets").toUtf8());
        }

        {
            /**
             * Since we package our own fontconfig into AppImage we should explicitly add
             * system configuration file into the search list. Otherwise it will not be found.
             *
             * Please note that FONTCONFIG_PATH should be set **before** we create our first
             * instance of QApplication for openGL probing, because it will make fontconfig
             * to be loaded before this environment variable set.
             */
            if (qgetenv("FONTCONFIG_PATH").isEmpty()) {
                const QString defaultFontsConfig = "/etc/fonts/fonts.conf";
                const QFileInfo info(defaultFontsConfig);
                if (info.exists()) {
                    qputenv("FONTCONFIG_PATH", QFile::encodeName(QDir::toNativeSeparators(info.absolutePath())));
                }
            }
        }
    }
#endif

    const QDir configPath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
    QSettings kritarc(configPath.absoluteFilePath("kritadisplayrc"), QSettings::IniFormat);

    QString root;
    QString language;
    {
        // Create a temporary application to get the root
        QCoreApplication app(argc, argv);
        Q_UNUSED(app);
        root = KoResourcePaths::getApplicationRoot();
        QSettings languageoverride(configPath.absoluteFilePath("klanguageoverridesrc"), QSettings::IniFormat);
        languageoverride.beginGroup("Language");
        language = languageoverride.value(qAppName(), "").toString();
    }

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

#if defined Q_OS_WIN || defined Q_OS_MACOS
    qputenv("QT_WIDGETS_RHI", "1");
    qputenv("QT_WIDGETS_RHI_BACKEND", "opengl");
    qputenv("QSG_RHI_BACKEND", "opengl");
#endif

#if defined Q_OS_WIN && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (config.rendererId() == KisOpenGL::RendererOpenGLES) {
        /**
         * Activate anti-flickering workarounds in Qt for ANGLE backend.
         * They should **not** be activated for openGL backend, because
         * it may cause flickering :)
         */
        if (!qEnvironmentVariableIsSet("QT_USE_PREMATURE_RESIZE_EVENTS")) {
            qInfo() << "INFO: activateing QT_USE_PREMATURE_RESIZE_EVENTS...";
            qputenv("QT_USE_PREMATURE_RESIZE_EVENTS", "1");
        }
        if (!qEnvironmentVariableIsSet("QT_ANGLE_MANUALLY_UPDATE_SURFACE_SIZE")) {
            qInfo() << "INFO: activateing QT_ANGLE_MANUALLY_UPDATE_SURFACE_SIZE...";
            qputenv("QT_ANGLE_MANUALLY_UPDATE_SURFACE_SIZE", "1");
        }
        if (!qEnvironmentVariableIsSet("QT_PREFILL_RHI_SURFACE")) {
            qInfo() << "INFO: activateing QT_PREFILL_RHI_SURFACE...";
            qputenv("QT_PREFILL_RHI_SURFACE", "1");
        }
    }
#endif

#if KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    if (!qEnvironmentVariableIsSet("QT_BACKING_STORE_USE_FAST_QIMAGE_TRANSFER")) {
        qputenv("QT_BACKING_STORE_USE_FAST_QIMAGE_TRANSFER", "1");
    }

    if (!qEnvironmentVariableIsSet("QT_FRAME_RATE_OVERRIDE")) {
        KisImageConfig cfg(true);
        if (!cfg.detectFpsLimit()) {
            qputenv("QT_FRAME_RATE_OVERRIDE", QString::number(cfg.fpsLimit()).toLatin1());
        }
    }
#endif

#ifdef Q_OS_WIN
        // HACK: https://bugs.kde.org/show_bug.cgi?id=390651
        resetRotation();
#endif
    }

    if (logUsage) {
        KisUsageLogger::initialize();
    }


#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    {
        QByteArray originalXdgDataDirs = qgetenv("XDG_DATA_DIRS");
        if (originalXdgDataDirs.isEmpty()) {
            // We don't want to completely override the default
            originalXdgDataDirs = "/usr/local/share/:/usr/share/";
        }

        // NOTE: This line helps also fontconfig have a user-accessible location on Android (see the commit).
        qputenv("XDG_DATA_DIRS", QString(QFile::encodeName(root + "share") + ":" + originalXdgDataDirs).toUtf8());
    }
#elif defined(Q_OS_HAIKU)
	qputenv("KRITA_PLUGIN_PATH", QString(QFile::encodeName(root + "lib")).toUtf8());
    qputenv("XDG_DATA_DIRS", QString(QFile::encodeName(root + "share") + ":" + qgetenv("XDG_DATA_DIRS")).toUtf8());
#else
    qputenv("XDG_DATA_DIRS", QFile::encodeName(QDir(root + "share").absolutePath()));
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

            if (!uiLanguages.empty()) {
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

    KisUsageLogger::writeLocaleSysInfo();

#if defined Q_OS_WIN && QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined QT5_HAS_WINTAB_SWITCH
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

#if defined Q_OS_WIN && QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    const bool forceWinTab = !KisConfig::useWin8PointerInputNoApp(&kritarc);
    using QWindowsApplication = QNativeInterface::Private::QWindowsApplication;
    if (auto nativeWindowsApp = dynamic_cast<QWindowsApplication *>(QGuiApplicationPrivate::platformIntegration())) {
        nativeWindowsApp->setWinTabEnabled(forceWinTab);
    }
#endif


    installTranslators(app);

    KisUsageLogger::writeHeader();
    KisOpenGL::initialize();

#ifdef HAVE_SET_HAS_BORDER_IN_FULL_SCREEN_DEFAULT
    if (QCoreApplication::testAttribute(Qt::AA_UseDesktopOpenGL)) {
        QWindowsWindowFunctions::setHasBorderInFullScreenDefault(true);
    }
#endif


    if (!language.isEmpty()) {
        if (rightToLeft) {
            KisApplication::setLayoutDirection(Qt::RightToLeft);
        }
        else {
            KisApplication::setLayoutDirection(Qt::LeftToRight);
        }
    }
#ifdef Q_OS_ANDROID
    KisApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif
    // Enable debugging translations from undeployed apps
    KLocalizedString::addDomainLocaleDir("krita", QDir(root + "share/locale").absolutePath());

    KLocalizedString::setApplicationDomain("krita");

    dbgLocale << "Available translations" << KLocalizedString::availableApplicationTranslations();
    dbgLocale << "Available domain translations" << KLocalizedString::availableDomainTranslations("krita");


#ifdef Q_OS_WIN
    QDir appdir(KoResourcePaths::getApplicationRoot());
    QString path = qgetenv("PATH");
    qputenv("PATH",
            QFile::encodeName(
                QDir::toNativeSeparators(appdir.absolutePath() + "/bin") + ";" +
                QDir::toNativeSeparators(appdir.absolutePath() + "/lib") + ";" +
                QDir::toNativeSeparators(appdir.absolutePath()) + ";" +
                path)
            );

    dbgKrita << "PATH" << qgetenv("PATH");
#endif

    if (KisApplication::applicationDirPath().contains(KRITA_BUILD_DIR)) {
        qFatal("FATAL: You're trying to run krita from the build location. You can only run Krita from the installation location.");
    }

#if defined HAVE_KCRASH
    KCrash::initialize();
#elif defined USE_DRMINGW
    tryInitDrMingw();
#endif
#if defined Q_OS_ANDROID
    // because we need qApp
    qputenv("MLT_REPOSITORY", QFile::encodeName(qApp->applicationDirPath()));

    QString loc;
    if (QStandardPaths::standardLocations(QStandardPaths::HomeLocation).size() > 1) {
        loc = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[1];
    } else {
        loc = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    }
    qputenv("MLT_DATA", QFile::encodeName(loc + "/share/mlt-7/"));
    qputenv("MLT_ROOT_DIR", QFile::encodeName(loc));
    qputenv("MLT_PROFILES_PATH", QFile::encodeName(loc + "/share/mlt-7/profiles/"));
    qputenv("MLT_PRESETS_PATH", QFile::encodeName(loc + "/share/mlt-7/presets/"));
    qputenv("MLT_PLUGIN_FILTER_STRING", "lib_mltplugin_");
#endif
    KisApplicationArguments args(app);

    if (app.isRunning()) {
        // only pass arguments to main instance if they are not for batch processing
        // any batch processing would be done in this separate instance
        const bool batchRun = args.exportAs() || args.exportSequence();

        if (!batchRun) {
            if (app.sendMessage(args.serialize().toBase64())) {
                return 0;
            }
        }
    }
#ifdef Q_OS_MACOS
    // HACK: Sandboxed macOS cannot use QSharedMemory on Qt<6
    else if (KisMacosEntitlements().sandbox()) {
        if(iskritaRunningActivate()) {
            return 0;
        }
    }
#endif

    if (!runningInKDE) {
        // Icons in menus are ugly and distracting
        KisApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    KisApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif
    app.installEventFilter(KisQtWidgetsTweaker::instance());
    app.setDesktopFileName(QStringLiteral("org.kde.krita"));

    if (!args.noSplash()) {
        QWidget *splash = new KisSplashScreen();
        app.setSplashScreen(splash);
    }

#if defined Q_OS_WIN
    KisConfig cfg(false);
    QOperatingSystemVersion osVersion = QOperatingSystemVersion::current();
    if (osVersion.type() == QOperatingSystemVersion::Windows) {
        // TODO QT6: update minimum requirement
        if (osVersion.majorVersion() < QOperatingSystemVersion::Windows7.majorVersion()) {
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined QT5_HAS_WINTAB_SWITCH
    // Check if WinTab/WinInk has actually activated
    const bool useWinInkAPI = !KisApplication::testAttribute(Qt::AA_MSWindowsUseWinTabAPI);

    if (useWinInkAPI != cfg.useWin8PointerInput()) {
        KisUsageLogger::log("WARNING: WinTab tablet protocol is not supported on this device. Switching to WinInk...");

        cfg.setUseWin8PointerInput(useWinInkAPI);
        cfg.setUseRightMiddleTabletButtonWorkaround(true);
    }
#endif
#endif
    KisApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents, false);

    // Set up remote arguments.
    QObject::connect(&app, &KisApplication::messageReceived,
                     &app, &KisApplication::remoteArguments);

    // Hardware information
    KisUsageLogger::writeSysInfo("\nHardware Information\n");
    KisUsageLogger::writeSysInfo(QString("  GPU Acceleration: %1").arg(kritarc.value("OpenGLRenderer", "auto").toString()));
    KisUsageLogger::writeSysInfo(QString("  Memory: %1 Mb").arg(KisImageConfig::totalRAM()));
    KisUsageLogger::writeSysInfo(QString("  Number of Cores: %1").arg(QThread::idealThreadCount()));
    KisUsageLogger::writeSysInfo(QString("  Swap Location: %1").arg(KisImageConfig(true).swapDir()));
    KisUsageLogger::writeSysInfo(
        QString("  Built for: %1")
            .arg(KisSupportedArchitectures::baseArchName()));
    KisUsageLogger::writeSysInfo(
        QString("  Base instruction set: %1")
            .arg(KisSupportedArchitectures::bestArchName()));
    KisUsageLogger::writeSysInfo(
        QString("  Supported instruction sets: %1")
            .arg(KisSupportedArchitectures::supportedInstructionSets()));
    KisUsageLogger::writeSysInfo("");

    KisConfig(true).logImportantSettings();

    KisApplication::setFont(KisUiFont::normalFont());

    if (!app.start(args)) {
        KisUsageLogger::log("Could not start Krita Application");
        return 1;
    }

    int state = KisApplication::exec();

    {
        QSettings kritarc(configPath.absoluteFilePath("kritadisplayrc"), QSettings::IniFormat);
        kritarc.setValue("canvasState", "OPENGL_SUCCESS");
    }

    if (logUsage) {
        KisUsageLogger::close();
    }

    KisPart::instance()->unloadPlaybackEngine();

#ifdef Q_OS_HAIKU
	kill(::getpid(), SIGKILL);
#endif

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
        KisApplication::removeTranslator(translator);
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
    KisApplication::installTranslator(translator);
}

void installQtTranslations(KisApplication &app)
{
    const QStringList qtCatalogs = {
        QStringLiteral("qt_"),
        QStringLiteral("qtbase_"),
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
                KisApplication::installTranslator(translator);
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
            const QString fullPath = QStringLiteral("assets:/") + subPath;
#else
            const QString root = QLibraryInfo::location(QLibraryInfo::PrefixPath);

            // Our patched k18n uses AppDataLocation (for AppImage). Not using
            // KoResourcePaths::getAppDataLocation is correct here, because we
            // need to look into the installation folder, not the configured appdata
            // folder.
            QString fullPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, subPath);

            if (fullPath.isEmpty()) {
                // ... but distro builds probably still use GenericDataLocation,
                // so check that too.
                fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, subPath);
            }

            if (fullPath.isEmpty()) {
                // And, failing all, use the deps install folder
                fullPath = root + "/share/" + subPath;
            }
#endif
            if (!QFile::exists(fullPath)) {
                continue;
            }

            QTranslator *translator = new QTranslator(&app);
            if (translator->load(fullPath)) {
                dbgLocale << "Loaded ECM translations for" << localeDirName << catalog;
                translator->setObjectName(QStringLiteral("QTranslator.%1.%2").arg(localeDirName, catalog));
                KisApplication::installTranslator(translator);
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
