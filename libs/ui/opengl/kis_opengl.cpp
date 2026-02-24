/*
 *  SPDX-FileCopyrightText: 2007 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2023 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <tuple>

#include <boost/optional.hpp>

#include <QtGlobal>

#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>

#include <QApplication>
#include <QScreen>
#include <QPixmapCache>
#include <QColorSpace>

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QVector>
#include <QWindow>
#include <QRegularExpression>
#include <QSettings>
#include <QScreen>

#include <klocalizedstring.h>

#include <KisRepaintDebugger.h>
#include <KisUsageLogger.h>
#include <kis_assert.h>
#include <kis_config.h>
#include <kis_debug.h>

#include <KisSurfaceColorSpaceWrapper.h>
#include "KisOpenGLModeProber.h"
#include "opengl/kis_opengl.h"

#include <config-hdr.h>
#include <config-use-surface-color-management-api.h>

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
/// openGL ES headers use a bit different names in prototypes,
/// add a workaround for it
#if !defined APIENTRYP && defined GL_APIENTRYP
#define APIENTRYP GL_APIENTRYP
#endif

typedef void (APIENTRYP PFNGLINVALIDATEBUFFERDATAPROC) (GLuint buffer);
#else
typedef void (QOPENGLF_APIENTRYP PFNGLINVALIDATEBUFFERDATAPROC) (GLuint buffer);
#endif

namespace
{
    // config option, set manually by main()
    bool g_isDebugSynchronous = false;

    bool g_sanityDefaultFormatIsSet = false;

    boost::optional<KisOpenGLModeProber::Result> openGLCheckResult;

    bool g_needsFenceWorkaround = false;

    QString g_surfaceFormatDetectionLog;
    QString g_debugText("OpenGL Info\n  **OpenGL not initialized**");

    QVector<KLocalizedString> g_openglWarningStrings;

    using DetectedRenderer = std::tuple<QString, QString, bool>;
    QVector<DetectedRenderer> g_detectedRenderers;
    KisOpenGL::OpenGLRenderers g_supportedRenderers;
    KisOpenGL::OpenGLRenderer g_rendererPreferredByQt;

    bool g_useBufferInvalidation = false;
    PFNGLINVALIDATEBUFFERDATAPROC g_glInvalidateBufferData = nullptr;

    bool g_forceDisableTextureBuffers = false;

    void overrideSupportedRenderers(KisOpenGL::OpenGLRenderers supportedRenderers, KisOpenGL::OpenGLRenderer preferredByQt) {
        g_supportedRenderers = supportedRenderers;
        g_rendererPreferredByQt = preferredByQt;
    }

    void appendOpenGLWarningString(KLocalizedString warning)
    {
        g_openglWarningStrings << warning;
    }

    void overrideOpenGLWarningString(QVector<KLocalizedString> warnings)
    {
        g_openglWarningStrings = warnings;
    }

    void openglOnMessageLogged(const QOpenGLDebugMessage& debugMessage) {
        qDebug() << "OpenGL:" << debugMessage;
    }

    KisOpenGL::OpenGLRenderer getRendererFromProbeResult(KisOpenGLModeProber::Result info) {

        KisOpenGL::OpenGLRenderer result = KisOpenGL::RendererDesktopGL;

        if (info.isOpenGLES()) {
            const QString rendererString = info.rendererString().toLower();

            if (rendererString.contains("basic render driver") ||
                rendererString.contains("software")) {

                result = KisOpenGL::RendererSoftware;
            } else {
                result = KisOpenGL::RendererOpenGLES;
            }
        }

        return result;
    }
}

void KisOpenGL::initialize()
{
    if (openGLCheckResult) return;

    KIS_SAFE_ASSERT_RECOVER_NOOP(g_sanityDefaultFormatIsSet);

    KisOpenGL::RendererConfig config;
    config.format = QSurfaceFormat::defaultFormat();

    openGLCheckResult =
        KisOpenGLModeProber::instance()->probeFormat(config, false);

#ifdef Q_OS_WIN

    if (!qEnvironmentVariableIsSet("KRITA_UNLOCK_TEXTURE_BUFFERS") &&
        openGLCheckResult->rendererString().toUpper().contains("ANGLE")) {

        // Angle should always be openGLES...
        KIS_SAFE_ASSERT_RECOVER_NOOP(KisOpenGL::hasOpenGLES());

        /**
         * Angle works badly with texture buffers on DirectX. It does no distinction
         * between stream and dynamic buffers, therefore it does too many copies of the
         * data:
         *
         * source user data -> ram buffer -> staging buffer -> native buffer -> texture,
         *
         * which is extremely slow when painting with big brushes. On Angle we should
         * just use normal unbuffered texture uploads
         */

        g_forceDisableTextureBuffers = true;
        appendOpenGLWarningString(
            ki18n("Texture buffers are explicitly disabled on ANGLE renderer due "
                  "to performance issues."));
    }
#endif


    g_debugText.clear();
    QDebug debugOut(&g_debugText);
    debugOut << "OpenGL Info\n";

    if (openGLCheckResult) {
        debugOut << "\n  Qt Platform Name: " << QGuiApplication::platformName();
        if (openGLCheckResult->xcbGlProviderProtocol()) {
            debugOut << "\n  Qt XCB GL integration plugin: "
                     << (*openGLCheckResult->xcbGlProviderProtocol() == KisOpenGL::XCB_EGL ? "xcb_egl" : "xcb_glx");
        }
        debugOut << "\n  Vendor: " << openGLCheckResult->vendorString();
        debugOut << "\n  Renderer: " << openGLCheckResult->rendererString();
        debugOut << "\n  Driver version: " << openGLCheckResult->driverVersionString();
        debugOut << "\n  Shading language: " << openGLCheckResult->shadingLanguageString();
        debugOut << "\n  Requested format: " << QSurfaceFormat::defaultFormat();
        debugOut << "\n  Current format: " << openGLCheckResult->format();
        {
            QDebugStateSaver saver(debugOut);
            debugOut.nospace() << "\n  GL version: " << openGLCheckResult->glMajorVersion() << "."
                               << openGLCheckResult->glMinorVersion();
        }
        debugOut << "\n  Supports deprecated functions" << openGLCheckResult->supportsDeprecatedFunctions();
        debugOut << "\n  Is OpenGL ES:" << openGLCheckResult->isOpenGLES();
        debugOut << "\n  supportsBufferMapping:" << openGLCheckResult->supportsBufferMapping();
        debugOut << "\n  supportsBufferInvalidation:" << openGLCheckResult->supportsBufferInvalidation();
        debugOut << "\n  forceDisableTextureBuffers:" << g_forceDisableTextureBuffers;
        debugOut << "\n  Extensions:";
        {
            QDebugStateSaver saver(debugOut);
            Q_FOREACH (const QByteArray &i, openGLCheckResult->extensions()) {
                debugOut.noquote() << "\n    " << QString::fromLatin1(i);
            }
        }
    }

    debugOut << "\n\nQPA OpenGL Detection Info";
    debugOut << "\n  supportsDesktopGL:" << bool(g_supportedRenderers & RendererDesktopGL);
#ifdef Q_OS_WIN
    debugOut << "\n  supportsAngleD3D11:" << bool(g_supportedRenderers & RendererOpenGLES);
    debugOut << "\n  isQtPreferAngle:" << bool(g_rendererPreferredByQt == RendererOpenGLES);
#else
    debugOut << "\n  supportsOpenGLES:" << bool(g_supportedRenderers & RendererOpenGLES);
    debugOut << "\n  isQtPreferOpenGLES:" << bool(g_rendererPreferredByQt == RendererOpenGLES);
#endif
    debugOut << "\n  Detected renderers:";
    {
        QDebugStateSaver saver(debugOut);
        Q_FOREACH (const DetectedRenderer &x, g_detectedRenderers) {
            debugOut.noquote().nospace() << "\n    " << (std::get<2>(x) ? "(Supported)" : "(Unsupported)") << " "
                                         << std::get<0>(x) << " (" << std::get<1>(x) << ") ";
        }
    }

//    debugOut << "\n== log ==\n";
//    debugOut.noquote();
//    debugOut << g_surfaceFormatDetectionLog;
//    debugOut.resetFormat();
//    debugOut << "\n== end log ==";

    dbgOpenGL.noquote().nospace() << g_debugText;
    KisUsageLogger::writeSysInfo(g_debugText);

    if (!openGLCheckResult) {
        return;
    }


    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif

    KisConfig cfg(true);

    g_useBufferInvalidation = cfg.readEntry("useBufferInvalidation", false);
    KisUsageLogger::writeSysInfo(QString("\nuseBufferInvalidation (config option): %1\n").arg(g_useBufferInvalidation ? "true" : "false"));

    if ((isOnX11 && openGLCheckResult->rendererString().startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
        g_needsFenceWorkaround = true;
    }

    /**
     * The large pixmap cache workaround was originally added to fix
     * the bug 361709 and later extended to all GPU/OS configurations.
     * This setting is still left here in case anyone finds the cached
     * method performing better that the direct drawing of assistants
     * onto the canvas.
     *
     * See bugs:
     *   https://bugs.kde.org/show_bug.cgi?id=361709
     *   https://bugs.kde.org/show_bug.cgi?id=401940
     */

    if (cfg.assistantsDrawMode() == KisConfig::ASSISTANTS_DRAW_MODE_LARGE_PIXMAP_CACHE) {
        const qreal devicePixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
        const QSize screenSize = QGuiApplication::primaryScreen()->size() * devicePixelRatio;
        const int minCacheSize = 20 * 1024;

        // reserve space for at least 4 textures
        const int cacheSize = 2048 + 5 * 4 * screenSize.width() * screenSize.height() / 1024; // KiB

        QPixmapCache::setCacheLimit(qMax(minCacheSize, cacheSize));
    }
}

void KisOpenGL::initializeContext(QOpenGLContext *ctx)
{
    KisConfig cfg(true);
    initialize();

    const bool isDebugEnabled = ctx->format().testOption(QSurfaceFormat::DebugContext);

    dbgUI << "OpenGL: Opening new context";
    if (isDebugEnabled) {
        // Passing ctx for ownership management only, not specifying context.
        // QOpenGLDebugLogger only function on the current active context.
        // FIXME: Do we need to make sure ctx is the active context?
        QOpenGLDebugLogger* openglLogger = new QOpenGLDebugLogger(ctx);
        if (openglLogger->initialize()) {
            qDebug() << "QOpenGLDebugLogger is initialized. Check whether you get a message below.";
            QObject::connect(openglLogger, &QOpenGLDebugLogger::messageLogged, &openglOnMessageLogged);
            openglLogger->startLogging(g_isDebugSynchronous ? QOpenGLDebugLogger::SynchronousLogging : QOpenGLDebugLogger::AsynchronousLogging);
            openglLogger->logMessage(QOpenGLDebugMessage::createApplicationMessage(QStringLiteral("QOpenGLDebugLogger is logging.")));
        } else {
            qDebug() << "QOpenGLDebugLogger cannot be initialized.";
            delete openglLogger;
        }
    }

    // Double check we were given the version we requested
    QSurfaceFormat format = ctx->format();
    QOpenGLFunctions *f = ctx->functions();
    f->initializeOpenGLFunctions();

    if (openGLCheckResult->supportsBufferInvalidation()) {
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        g_glInvalidateBufferData = (PFNGLINVALIDATEBUFFERDATAPROC)ctx->getProcAddress("glInvalidateBufferData");
    }

    QFile log(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/krita-opengl.txt");
    log.open(QFile::WriteOnly);
    QString vendor((const char*)f->glGetString(GL_VENDOR));
    log.write(vendor.toLatin1());
    log.write(", ");
    log.write(openGLCheckResult->rendererString().toLatin1());
    log.write(", ");
    QString version((const char*)f->glGetString(GL_VERSION));
    log.write(version.toLatin1());
    log.close();
}

const QString &KisOpenGL::getDebugText()
{
    initialize();
    return g_debugText;
}

QStringList KisOpenGL::getOpenGLWarnings() {
    QStringList strings;
    Q_FOREACH (const KLocalizedString &item, g_openglWarningStrings) {
        strings << item.toString();
    }
    return strings;
}

QString KisOpenGL::currentDriver()
{
    initialize();
    if (openGLCheckResult) {
        return openGLCheckResult->driverVersionString();
    }
    return QString();
}

// Check whether we can allow LoD on OpenGL3 without triggering
// all of the other 3.2 functionality.
bool KisOpenGL::supportsLoD()
{
    initialize();
    return openGLCheckResult && openGLCheckResult->supportsLoD();
}

bool KisOpenGL::hasOpenGL3()
{
    initialize();
    return openGLCheckResult && openGLCheckResult->hasOpenGL3();
}

bool KisOpenGL::supportsVAO()
{
    initialize();
    return openGLCheckResult && openGLCheckResult->supportsVAO();
}

bool KisOpenGL::hasOpenGLES()
{
    initialize();
    return openGLCheckResult && openGLCheckResult->isOpenGLES();
}

bool KisOpenGL::supportsFenceSync()
{
    initialize();
    return openGLCheckResult && openGLCheckResult->supportsFenceSync();
}

bool KisOpenGL::supportsBufferMapping()
{
    initialize();
    return openGLCheckResult && openGLCheckResult->supportsBufferMapping();
}

bool KisOpenGL::forceDisableTextureBuffers()
{
    initialize();
    return g_forceDisableTextureBuffers;
}

bool KisOpenGL::shouldUseTextureBuffers(bool userPreference)
{
    initialize();
    return !g_forceDisableTextureBuffers && userPreference;
}

bool KisOpenGL::useTextureBufferInvalidation()
{
    initialize();
    return g_useBufferInvalidation &&
        openGLCheckResult && openGLCheckResult->supportsBufferInvalidation();
}

bool KisOpenGL::useFBOForToolOutlineRendering()
{
    initialize();
    return openGLCheckResult && openGLCheckResult->supportsFBO();
}

std::optional<KisOpenGL::XcbGLProviderProtocol> KisOpenGL::xcbGlProviderProtocol()
{
    initialize();
    return openGLCheckResult ? openGLCheckResult->xcbGlProviderProtocol() : std::nullopt;
}

bool KisOpenGL::needsFenceWorkaround()
{
    initialize();
    return g_needsFenceWorkaround;
}

void KisOpenGL::testingInitializeDefaultSurfaceFormat()
{
    setDefaultSurfaceConfig(selectSurfaceConfig(KisOpenGL::RendererAuto, KisConfig::BT709_G22, KisConfig::CanvasSurfaceBitDepthMode::DepthAuto, false));
}

void KisOpenGL::setDebugSynchronous(bool value)
{
    g_isDebugSynchronous = value;
}

void KisOpenGL::glInvalidateBufferData(uint buffer)
{
    g_glInvalidateBufferData(buffer);
}

KisOpenGL::OpenGLRenderer KisOpenGL::getCurrentOpenGLRenderer()
{
    if (!openGLCheckResult) return RendererAuto;
    return getRendererFromProbeResult(*openGLCheckResult);
}

KisOpenGL::OpenGLRenderer KisOpenGL::getQtPreferredOpenGLRenderer()
{
    return g_rendererPreferredByQt;
}

KisOpenGL::OpenGLRenderers KisOpenGL::getSupportedOpenGLRenderers()
{
    return g_supportedRenderers;
}

KisOpenGL::OpenGLRenderer KisOpenGL::getUserPreferredOpenGLRendererConfig()
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
    return convertConfigToOpenGLRenderer(kritarc.value("OpenGLRenderer", "auto").toString());
}

void KisOpenGL::setUserPreferredOpenGLRendererConfig(KisOpenGL::OpenGLRenderer renderer)
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
    kritarc.setValue("OpenGLRenderer", KisOpenGL::convertOpenGLRendererToConfig(renderer));
}

QString KisOpenGL::convertOpenGLRendererToConfig(KisOpenGL::OpenGLRenderer renderer)
{
    switch (renderer) {
    case RendererNone:
        return QStringLiteral("none");
    case RendererSoftware:
        return QStringLiteral("software");
    case RendererDesktopGL:
        return QStringLiteral("desktop");
    case RendererOpenGLES:
        return QStringLiteral("angle");
    default:
        return QStringLiteral("auto");
    }
}

KisOpenGL::OpenGLRenderer KisOpenGL::convertConfigToOpenGLRenderer(QString renderer)
{
    if (renderer == "desktop") {
        return RendererDesktopGL;
    } else if (renderer == "angle") {
        return RendererOpenGLES;
    } else if (renderer == "software") {
        return RendererSoftware;
    } else if (renderer == "none") {
        return RendererNone;
    } else {
        return RendererAuto;
    }
}

KisOpenGL::OpenGLRenderer KisOpenGL::RendererConfig::rendererId() const
{
    KisOpenGL::OpenGLRenderer result = RendererAuto;

    if (format.renderableType() == QSurfaceFormat::OpenGLES &&
        angleRenderer == AngleRendererD3d11Warp) {

        result = RendererSoftware;

    } else if (format.renderableType() == QSurfaceFormat::OpenGLES) {
        // If D3D11, D3D9?, Default (which is after probing, if selected)
        // or the system specifies QT_OPENGL_ES_2
        result = RendererOpenGLES;
    } else if (format.renderableType() == QSurfaceFormat::OpenGL) {
        result = RendererDesktopGL;
    } else if (format.renderableType() == QSurfaceFormat::DefaultRenderableType &&
               angleRenderer == AngleRendererD3d11) {
        // noop
    } else {
        qWarning() << "WARNING: unsupported combination of OpenGL renderer" << ppVar(format.renderableType()) << ppVar(angleRenderer);
    }

    return result;
}

namespace {

typedef std::pair<QSurfaceFormat::RenderableType, KisOpenGL::AngleRenderer> RendererInfo;

RendererInfo getRendererInfo(KisOpenGL::OpenGLRenderer renderer)
{
    RendererInfo info = {QSurfaceFormat::DefaultRenderableType,
                         KisOpenGL::AngleRendererD3d11};

    switch (renderer) {
    case KisOpenGL::RendererNone:
        info = {QSurfaceFormat::DefaultRenderableType, KisOpenGL::AngleRendererDefault};
        break;
    case KisOpenGL::RendererAuto:
        break;
    case KisOpenGL::RendererDesktopGL:
        info = {QSurfaceFormat::OpenGL, KisOpenGL::AngleRendererDefault};
        break;
    case KisOpenGL::RendererOpenGLES:
        info = {QSurfaceFormat::OpenGLES, KisOpenGL::AngleRendererD3d11};
        break;
    case KisOpenGL::RendererSoftware:
        info = {QSurfaceFormat::OpenGLES, KisOpenGL::AngleRendererD3d11Warp};
        break;
    }

    return info;
}

QOpenGLContext::OpenGLModuleType determineOpenGLImplementation(const RendererInfo &info)
{
    switch (info.first) {
    case QSurfaceFormat::OpenGLES:
#if defined(Q_OS_WINDOWS)
        // https://invent.kde.org/szaman/qtbase/-/blob/krita/5.15/src/plugins/platforms/windows/qwindowsintegration.cpp#L425
        switch (info.second) {
        case KisOpenGL::AngleRendererD3d11:
        case KisOpenGL::AngleRendererD3d9:
        case KisOpenGL::AngleRendererD3d11Warp:
            return QOpenGLContext::LibGLES;
        // Assume system OpenGL -- QOpenGLStaticContext
        default:
            break;
        }
        return QOpenGLContext::LibGL;
#else
        // At least Manjaro Qt can perfectly call up a ES context,
        // while Qt says via macros that it doesn't support that...
        return QOpenGLContext::LibGLES;
#endif
    case QSurfaceFormat::DefaultRenderableType:
#ifdef Q_OS_WIN
    // https://invent.kde.org/szaman/qtbase/-/blob/krita/5.15/src/plugins/platforms/windows/qwindowsglcontext.cpp#L1117
        return QOpenGLContext::LibGL;
#else
    // https://invent.kde.org/szaman/qtbase/-/blob/krita/5.15/src/plugins/platforms/xcb/gl_integrations/xcb_glx/qglxintegration.cpp#L246
#if defined(QT_OPENGL_ES_2)
    return QOpenGLContext::LibGLES;
#else
    return QOpenGLContext::LibGL;
#endif
#endif
    case QSurfaceFormat::OpenGL:
    default:
        // https://invent.kde.org/szaman/qtbase/-/blob/krita/5.15/src/plugins/platforms/windows/qwindowsglcontext.cpp#L1117
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(info.first != QSurfaceFormat::OpenVG, QOpenGLContext::LibGL);
        // https://invent.kde.org/szaman/qtbase/-/blob/krita/5.15/src/gui/kernel/qplatformintegration.cpp#L547
        return QOpenGLContext::LibGL;
    };
}

KisOpenGL::RendererConfig generateSurfaceConfig(KisOpenGL::OpenGLRenderer renderer,
                                                std::pair<KisSurfaceColorSpaceWrapper, int> rootSurfaceFormat,
                                                bool debugContext,
                                                bool inhibitCompatibilityProfile)
{
    RendererInfo info = getRendererInfo(renderer);

    KisOpenGL::RendererConfig config;
    config.angleRenderer = info.second;

    
    dbgOpenGL << "Requesting configuration for" << info.first << info.second;
    dbgOpenGL << "Requesting root surface format" << rootSurfaceFormat;

    QSurfaceFormat &format = config.format;
    const auto openGLModuleType = determineOpenGLImplementation(info);
    switch (openGLModuleType) {
    case QOpenGLContext::LibGL:
#if defined Q_OS_MACOS
        format.setVersion(4, 1);
        format.setProfile(QSurfaceFormat::CoreProfile);
#else
        // If asked for 3.0 "Core", Qt will instead request
        // an OpenGL ES context.
        // NVIDIA's GLX implementation will not allow that and results
        // in a forced process exit through X11 (NVIDIA bug #3959482).
        format.setVersion(3, 3);
        // Make sure to request a Compatibility profile to have NVIDIA
        // return the maximum supported GL version.

        if (!inhibitCompatibilityProfile) {
            format.setProfile(QSurfaceFormat::CompatibilityProfile);
        }
#ifdef Q_OS_WIN
        // Some parts of Qt seems to require deprecated functions. On Windows
        // with the Intel Graphics driver, things like canvas decorations and
        // the Touch Docker does not render without this option.
        format.setOptions(QSurfaceFormat::DeprecatedFunctions);
#endif
#endif
        break;
    case QOpenGLContext::LibGLES:
        format.setVersion(3, 0);
        format.setProfile(QSurfaceFormat::NoProfile);
        break;
    }

    dbgOpenGL << "Version selected:" << openGLModuleType << format.version();

    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);

    KisOpenGLModeProber::initSurfaceFormatFromConfig(rootSurfaceFormat, &format);

    format.setRenderableType(info.first);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // Disable vertical refresh syncing
    if (KisRepaintDebugger::enabled()) {
        // With repaint debugging, vsync is preferred so that all update regions
        // can be visible.
        format.setSwapInterval(1);
    }
    if (debugContext) {
        format.setOption(QSurfaceFormat::DebugContext, true);
    }

    return config;
}

bool isOpenGLRendererBlacklisted(const QString &rendererString,
                                 const QString &driverVersionString,
                                 QVector<KLocalizedString> *warningMessage)
{
    bool isBlacklisted = false;
#ifndef Q_OS_WIN
    Q_UNUSED(rendererString);
    Q_UNUSED(driverVersionString);
    Q_UNUSED(warningMessage);
#else
    // Special blacklisting of OpenGL/ANGLE is tracked on:
    // https://phabricator.kde.org/T7411

    // HACK: Specifically detect for Intel driver build number
    //       See https://www.intel.com/content/www/us/en/support/articles/000005654/graphics.html
    if (rendererString.startsWith("Intel")) {
        KLocalizedString knownBadIntelWarning = ki18n("The Intel graphics driver in use is known to have issues with OpenGL.");
        KLocalizedString grossIntelWarning = ki18n(
            "Intel graphics drivers tend to have issues with OpenGL so ANGLE will be used by default. "
            "You may manually switch to OpenGL but it is not guaranteed to work properly."
        );
        QRegularExpression regex("\\b\\d{1,2}\\.\\d{1,2}\\.(\\d{1,3})\\.(\\d{4})\\b");
        QRegularExpressionMatch match = regex.match(driverVersionString);
        if (match.hasMatch()) {
            const int thirdPart = match.captured(1).toInt();
            const int fourthPart = match.captured(2).toInt();
            int driverBuild;
            if (thirdPart >= 100) {
                driverBuild = thirdPart * 10000 + fourthPart;
            } else {
                driverBuild = fourthPart;
            }
            qDebug() << "Detected Intel driver build number as" << driverBuild;
            if (driverBuild > 4636 && driverBuild < 4729) {
                // Make ANGLE the preferred renderer for Intel driver versions
                // between build 4636 and 4729 (exclusive) due to an UI offset bug.
                // See https://communities.intel.com/thread/116003
                // (Build 4636 is known to work from some test results)
                qDebug() << "Detected Intel driver build between 4636 and 4729, making ANGLE the preferred renderer";
                isBlacklisted = true;
                *warningMessage << knownBadIntelWarning;
            } else if (driverBuild == 4358) {
                // There are several reports on a bug where the canvas is not being
                // updated properly which has debug info pointing to this build.
                qDebug() << "Detected Intel driver build 4358, making ANGLE the preferred renderer";
                isBlacklisted = true;
                *warningMessage << knownBadIntelWarning;
            }
        } else {
            // In case Intel changed the driver version format to something that
            // we don't understand, we still select ANGLE.
            qDebug() << "Detected Intel driver with unknown version format, making ANGLE the preferred renderer";
            isBlacklisted = true;
            *warningMessage << grossIntelWarning;
        }
    }
#endif
    return isBlacklisted;
}

boost::optional<bool> orderPreference(bool lhs, bool rhs)
{
    if (lhs == rhs) return boost::none;
    if (lhs && !rhs) return true;
    if (!lhs && rhs) return false;
    return false;
}

#define ORDER_BY(lhs, rhs) if (auto res = orderPreference((lhs), (rhs))) { return *res; }

class FormatPositionLess
{
public:

    FormatPositionLess()
    {
    }

    bool operator()(const KisOpenGL::RendererConfig &lhs, const KisOpenGL::RendererConfig &rhs) const {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_preferredColorSpace != KisSurfaceColorSpaceWrapper::DefaultColorSpace);
        if (m_preferredRendererByUser != KisOpenGL::RendererSoftware) {
            ORDER_BY(!isFallbackOnly(lhs.rendererId()), !isFallbackOnly(rhs.rendererId()));
        }

        ORDER_BY(isPreferredColorSpace(lhs.format),
                 isPreferredColorSpace(rhs.format));

        if (doPreferHDR()) {
            ORDER_BY(isHDRFormat(lhs.format), isHDRFormat(rhs.format));
        } else {
            ORDER_BY(!isHDRFormat(lhs.format), !isHDRFormat(rhs.format));
        }

        if (m_preferredRendererByUser != KisOpenGL::RendererAuto) {
            ORDER_BY(lhs.rendererId() == m_preferredRendererByUser,
                     rhs.rendererId() == m_preferredRendererByUser);
        }

        ORDER_BY(!isBlacklisted(lhs.rendererId()), !isBlacklisted(rhs.rendererId()));

        if (doPreferHDR() &&
            m_preferredRendererByHDR != KisOpenGL::RendererAuto) {

            ORDER_BY(lhs.rendererId() == m_preferredRendererByHDR,
                     rhs.rendererId() == m_preferredRendererByHDR);

        }

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_preferredRendererByQt != KisOpenGL::RendererAuto);

        ORDER_BY(lhs.rendererId() == m_preferredRendererByQt,
                 rhs.rendererId() == m_preferredRendererByQt);

        ORDER_BY(lhs.format.redBufferSize() == m_userPreferredBitDepth,
                 rhs.format.redBufferSize() == m_userPreferredBitDepth);

        return false;
    }


public:
    void setPreferredColorSpace(const KisSurfaceColorSpaceWrapper &preferredColorSpace) {
        m_preferredColorSpace = preferredColorSpace;
    }

    void setPreferredRendererByQt(const KisOpenGL::OpenGLRenderer &preferredRendererByQt) {
        m_preferredRendererByQt = preferredRendererByQt;
    }

    void setPreferredRendererByUser(const KisOpenGL::OpenGLRenderer &preferredRendererByUser) {
        m_preferredRendererByUser = preferredRendererByUser;
    }

    void setPreferredRendererByHDR(const KisOpenGL::OpenGLRenderer &preferredRendererByHDR) {
        m_preferredRendererByHDR = preferredRendererByHDR;
    }

    void setOpenGLBlacklisted(bool openGLBlacklisted) {
        m_openGLBlacklisted = openGLBlacklisted;
    }

    void setOpenGLESBlacklisted(bool openGLESBlacklisted) {
        m_openGLESBlacklisted = openGLESBlacklisted;
    }

    void setUserPreferredBitDepth(int value) {
        m_userPreferredBitDepth = value;
    }

    bool isOpenGLBlacklisted() const {
        return m_openGLBlacklisted;
    }

    bool isOpenGLESBlacklisted() const {
        return m_openGLESBlacklisted;
    }

    KisSurfaceColorSpaceWrapper preferredColorSpace() const {
        return m_preferredColorSpace;
    }

    KisOpenGL::OpenGLRenderer preferredRendererByUser() const {
        return m_preferredRendererByUser;
    }

    int userPreferredBitDepth() const {
        return m_userPreferredBitDepth;
    }

private:
    bool isHDRFormat(const QSurfaceFormat &f) const {
#ifdef HAVE_HDR
        return f.colorSpace() == KisSurfaceColorSpaceWrapper::makeBt2020PQColorSpace() ||
            f.colorSpace() == KisSurfaceColorSpaceWrapper::makeSCRGBColorSpace();
#else
        Q_UNUSED(f);
        return false;
#endif
    }

    bool isFallbackOnly(KisOpenGL::OpenGLRenderer r) const {
        return r == KisOpenGL::RendererSoftware;
    }

    bool isBlacklisted(KisOpenGL::OpenGLRenderer r) const {
        KIS_SAFE_ASSERT_RECOVER_NOOP(r == KisOpenGL::RendererAuto ||
                                     r == KisOpenGL::RendererDesktopGL ||
                                     r == KisOpenGL::RendererOpenGLES ||
                                     r == KisOpenGL::RendererSoftware ||
                                     r == KisOpenGL::RendererNone);

        return (r == KisOpenGL::RendererDesktopGL && m_openGLBlacklisted) ||
            (r == KisOpenGL::RendererOpenGLES && m_openGLESBlacklisted) ||
            (r == KisOpenGL::RendererSoftware && m_openGLESBlacklisted);
    }

    bool doPreferHDR() const {
#ifdef HAVE_HDR
        return m_preferredColorSpace == KisSurfaceColorSpaceWrapper::bt2020PQColorSpace ||
            m_preferredColorSpace == KisSurfaceColorSpaceWrapper::scRGBColorSpace;
#else
        return false;
#endif
    }

    bool isPreferredColorSpace(const QSurfaceFormat & surfaceFormat) const {
        return KisOpenGLModeProber::fuzzyCompareColorSpaces(
            m_preferredColorSpace, 
            KisSurfaceColorSpaceWrapper::fromQtColorSpace(surfaceFormat.colorSpace()));
    }

private:
    KisSurfaceColorSpaceWrapper m_preferredColorSpace;
    KisOpenGL::OpenGLRenderer m_preferredRendererByQt = KisOpenGL::RendererDesktopGL;
    KisOpenGL::OpenGLRenderer m_preferredRendererByUser = KisOpenGL::RendererAuto;
    KisOpenGL::OpenGLRenderer m_preferredRendererByHDR = KisOpenGL::RendererAuto;
    bool m_openGLBlacklisted = false;
    bool m_openGLESBlacklisted = false;
    int m_userPreferredBitDepth = 8;
};

struct DetectionDebug : public QDebug
{
    DetectionDebug(QString *string)
        : QDebug(string),
          m_string(string),
          m_originalSize(string->size())
    {}
    ~DetectionDebug() { dbgOpenGL << m_string->right(m_string->size() - m_originalSize); *this << Qt::endl; }

    QString *m_string;
    int m_originalSize;
};
}

#define dbgDetection() DetectionDebug(&g_surfaceFormatDetectionLog)

KisOpenGL::RendererConfig KisOpenGL::selectSurfaceConfig(KisOpenGL::OpenGLRenderer preferredRenderer,
                                                         KisConfig::RootSurfaceFormat preferredRootSurfaceFormat,
                                                         KisConfig::CanvasSurfaceBitDepthMode preferredCanvasSurfaceBitMode,
                                                         bool enableDebug)
{
    QVector<KLocalizedString> warningMessages;

    using Info = boost::optional<KisOpenGLModeProber::Result>;

    QHash<OpenGLRenderer, Info> renderersToTest;
#ifndef Q_OS_ANDROID
    renderersToTest.insert(RendererDesktopGL, Info());
#endif
    renderersToTest.insert(RendererOpenGLES, Info());

#ifdef Q_OS_WIN
    renderersToTest.insert(RendererSoftware, Info());
#endif

    auto makeDefaultSurfaceFormatPair = [] () -> std::pair<KisSurfaceColorSpaceWrapper, int> {
        return {KisSurfaceColorSpaceWrapper::DefaultColorSpace, 8};
    };

#if defined HAVE_HDR
    std::vector<std::pair<KisSurfaceColorSpaceWrapper, int>> formatSymbolPairs(
        {
            // TODO: check if we can use real sRGB space here
            {KisSurfaceColorSpaceWrapper::DefaultColorSpace, 8},
            {KisSurfaceColorSpaceWrapper::scRGBColorSpace, 16},
            {KisSurfaceColorSpaceWrapper::bt2020PQColorSpace, 10}
        });
#elif KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    std::vector<std::pair<KisSurfaceColorSpaceWrapper, int>> formatSymbolPairs(
        {
            {KisSurfaceColorSpaceWrapper::DefaultColorSpace, 8},
            {KisSurfaceColorSpaceWrapper::DefaultColorSpace, 10},
        });
#else
    std::vector<std::pair<KisSurfaceColorSpaceWrapper, int>> formatSymbolPairs(
        {
            {KisSurfaceColorSpaceWrapper::DefaultColorSpace, 8},
        });
#endif

    bool shouldInhibitCompatibilityProfile = false;
    KisOpenGL::RendererConfig defaultConfig = generateSurfaceConfig(KisOpenGL::RendererAuto,
                                                                    makeDefaultSurfaceFormatPair(),
                                                                    false,
                                                                    shouldInhibitCompatibilityProfile);
    Info info = KisOpenGLModeProber::instance()->probeFormat(defaultConfig);

#ifndef Q_OS_MACOS
    // When RendererAuto is active, Qt may perform insane things internally,
    // e.g. switch from OpenGL to OpenGLES automatically. And the presence of
    // the compatibility profile flag will cause the context creation process
    // to fail.
    //
    // So, here we request an explicit API again to avoid Qt making decisions
    // for us.

    if (!info) {
        dbgOpenGL << "Failed to probe default Qt's openGL format.. Trying DesktopGL with compatibility enabled...";
        shouldInhibitCompatibilityProfile = false;
        defaultConfig = generateSurfaceConfig(KisOpenGL::RendererDesktopGL,
                                              makeDefaultSurfaceFormatPair(),
                                              false,
                                              shouldInhibitCompatibilityProfile);
        info = KisOpenGLModeProber::instance()->probeFormat(defaultConfig);
    }

    if (!info) {
        dbgOpenGL << "Failed again.. Trying DesktopGL with compatibility disabled...";
        shouldInhibitCompatibilityProfile = true;
        defaultConfig = generateSurfaceConfig(KisOpenGL::RendererDesktopGL,
                                              makeDefaultSurfaceFormatPair(),
                                              false,
                                              shouldInhibitCompatibilityProfile);
        info = KisOpenGLModeProber::instance()->probeFormat(defaultConfig);
    }

    if (!info) {
        dbgOpenGL << "Failed again.. Trying OpenGLES...";
        shouldInhibitCompatibilityProfile = false;
        defaultConfig = generateSurfaceConfig(KisOpenGL::RendererOpenGLES,
                                              makeDefaultSurfaceFormatPair(),
                                              false,
                                              true);
        info = KisOpenGLModeProber::instance()->probeFormat(defaultConfig);
    }

#endif /* Q_OS_MACOS */

#ifdef Q_OS_WIN
    if (!info) {
        // try software rasterizer (WARP)
        defaultConfig = generateSurfaceConfig(KisOpenGL::RendererSoftware,
                                              makeDefaultSurfaceFormatPair(),
                                              false,
                                              shouldInhibitCompatibilityProfile);
        info = KisOpenGLModeProber::instance()->probeFormat(defaultConfig);

        if (!info) {
            renderersToTest.remove(RendererSoftware);
        }
    }
#endif

    if (!info) {
        dbgOpenGL << "Failed to probe default openGL format! No openGL support will be available in Krita";
        return KisOpenGL::RendererConfig();
    }

    const OpenGLRenderer defaultRenderer = getRendererFromProbeResult(*info);

    /**
     * On Windows we always prefer Angle, not what Qt suggests us
     */
#ifdef Q_OS_WIN
    const OpenGLRenderer preferredAutoRenderer = RendererOpenGLES;
#else
    const OpenGLRenderer preferredAutoRenderer = defaultRenderer;
#endif

    OpenGLRenderers supportedRenderers = RendererNone;

    FormatPositionLess compareOp;
    compareOp.setPreferredRendererByQt(preferredAutoRenderer);

#ifdef HAVE_HDR
    compareOp.setPreferredColorSpace(
        preferredRootSurfaceFormat == KisConfig::BT709_G22 ? KisSurfaceColorSpaceWrapper::sRGBColorSpace :
        preferredRootSurfaceFormat == KisConfig::BT709_G10 ? KisSurfaceColorSpaceWrapper::scRGBColorSpace :
        KisSurfaceColorSpaceWrapper::bt2020PQColorSpace);
#else
    Q_UNUSED(preferredRootSurfaceFormat);
    compareOp.setPreferredColorSpace(KisSurfaceColorSpaceWrapper::sRGBColorSpace);
#endif

#ifdef Q_OS_WIN
    compareOp.setPreferredRendererByHDR(KisOpenGL::RendererOpenGLES);
#endif
    compareOp.setPreferredRendererByUser(preferredRenderer);
    compareOp.setOpenGLESBlacklisted(false); // We cannot blacklist ES drivers atm

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    // 10-bit is the default, 8-bit is set explicitly by the user
    compareOp.setUserPreferredBitDepth(preferredCanvasSurfaceBitMode == KisConfig::CanvasSurfaceBitDepthMode::Depth8Bit ? 8 : 10);
#else
    Q_UNUSED(preferredCanvasSurfaceBitMode)
#endif

    renderersToTest[defaultRenderer] = info;

    for (auto it = renderersToTest.begin(); it != renderersToTest.end(); ++it) {
        Info info = it.value();

        if (!info) {
            const RendererConfig config = generateSurfaceConfig(it.key(), makeDefaultSurfaceFormatPair(), false, shouldInhibitCompatibilityProfile);
            dbgOpenGL << "Probing" << it.key() << "from default:" << config.format << config.angleRenderer
                      << config.rendererId();
            info = KisOpenGLModeProber::instance()->probeFormat(config);
            *it = info;
        } else {
            dbgOpenGL << "Already probed:" << it.key();
        }

        compareOp.setOpenGLBlacklisted(
            !info ||
            isOpenGLRendererBlacklisted(info->rendererString(),
                                        info->driverVersionString(),
                                        &warningMessages));

        if (info) {
            dbgOpenGL << "Result:" << info->rendererString() << info->driverVersionString()
                      << info->isSupportedVersion();
        }

        if (info) {
            g_detectedRenderers << std::make_tuple(info->rendererString(),
                                                   info->driverVersionString(),
                                                   info->isSupportedVersion());
        }

        if (info && info->isSupportedVersion()) {
            supportedRenderers |= it.key();
        }
    }

    OpenGLRenderer preferredByQt = preferredAutoRenderer;

    if (preferredByQt == RendererDesktopGL &&
        supportedRenderers & RendererDesktopGL &&
        compareOp.isOpenGLBlacklisted()) {

        preferredByQt = RendererOpenGLES;

    } else if (preferredByQt == RendererOpenGLES &&
               supportedRenderers & RendererOpenGLES &&
               compareOp.isOpenGLESBlacklisted()) {

        preferredByQt = RendererDesktopGL;
    }

    QVector<RendererConfig> preferredConfigs;
    for (auto it = renderersToTest.begin(); it != renderersToTest.end(); ++it) {
        // if default mode of the renderer doesn't work, then custom won't either
        if (!it.value()) continue;

        Q_FOREACH (const auto &formatPair, formatSymbolPairs) {
            preferredConfigs << generateSurfaceConfig(it.key(), formatPair, enableDebug, shouldInhibitCompatibilityProfile);
        }
    }

    std::stable_sort(preferredConfigs.begin(), preferredConfigs.end(), compareOp);

    dbgDetection() << "Supported renderers:" << supportedRenderers;

    dbgDetection() << "Surface format preference list:";
    Q_FOREACH (const KisOpenGL::RendererConfig &config, preferredConfigs) {
        dbgDetection() << "*" << config.format;
        dbgDetection() << "   " << config.rendererId();
    }

    KisOpenGL::RendererConfig resultConfig = defaultConfig;

    if (preferredRenderer != RendererNone) {
        Q_FOREACH (const KisOpenGL::RendererConfig &config, preferredConfigs) {
            dbgDetection() <<"Probing format..." << config.format.colorSpace() << config.rendererId();
            Info info = KisOpenGLModeProber::instance()->probeFormat(config);

            if (info && info->isSupportedVersion()) {

#ifdef Q_OS_WIN
                // HACK: Block ANGLE with Direct3D9
                //       Direct3D9 does not give OpenGL ES 3.0
                //       Some versions of ANGLE returns OpenGL version 3.0 incorrectly

                if (info->isUsingAngle() &&
                        info->rendererString().contains("Direct3D9", Qt::CaseInsensitive)) {

                    dbgDetection() << "Skipping Direct3D 9 Angle implementation, it shouldn't have happened.";

                    continue;
                }
#endif

                dbgDetection() << "Found format:" << config.format;
                dbgDetection() << "   " << config.rendererId();

                resultConfig = config;
                break;
            }
        }

        {
            const bool colorSpaceIsCorrect =
                    KisOpenGLModeProber::fuzzyCompareColorSpaces(compareOp.preferredColorSpace(),
                                                                 KisSurfaceColorSpaceWrapper::fromQtColorSpace(resultConfig.format.colorSpace()));

            const bool rendererIsCorrect =
                    compareOp.preferredRendererByUser() == KisOpenGL::RendererAuto ||
                    compareOp.preferredRendererByUser() == resultConfig.rendererId();

            if (!rendererIsCorrect && colorSpaceIsCorrect) {
                warningMessages << ki18n("Preferred renderer doesn't support requested surface format. Another renderer has been selected.");
            } else if (!colorSpaceIsCorrect) {
                warningMessages << ki18n("Preferred output format is not supported by available renderers");
            }

        }
    } else {
        resultConfig.format = QSurfaceFormat();
        resultConfig.angleRenderer = AngleRendererDefault;
    }

    overrideSupportedRenderers(supportedRenderers, preferredByQt);
    overrideOpenGLWarningString(warningMessages);

    return resultConfig;
}

void KisOpenGL::setDefaultSurfaceConfig(const KisOpenGL::RendererConfig &config)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!g_sanityDefaultFormatIsSet);

    g_sanityDefaultFormatIsSet = true;
    QSurfaceFormat::setDefaultFormat(config.format);

    if (config.format.renderableType() == QSurfaceFormat::OpenGLES) {
        QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
#ifdef Q_OS_WIN
        if (!qEnvironmentVariableIsSet("QT_ANGLE_PLATFORM")) {
            // Force ANGLE to use Direct3D11. D3D9 doesn't support OpenGL ES 3 and WARP
            //  might get weird crashes atm.
            qputenv("QT_ANGLE_PLATFORM", KisOpenGLModeProber::angleRendererToString(config.angleRenderer).toLatin1());
        }
#endif
    } else if (config.format.renderableType() == QSurfaceFormat::OpenGL) {
        QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
    }
}

bool KisOpenGL::hasOpenGL()
{
    return openGLCheckResult->isSupportedVersion();
}
