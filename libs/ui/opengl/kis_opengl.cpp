/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include <config-hdr.h>
#include "opengl/kis_opengl.h"
#include "opengl/kis_opengl_p.h"

#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QVector>
#include <QWindow>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_config.h>
#include "KisOpenGLModeProber.h"

#include <KisUsageLogger.h>
#include <boost/optional.hpp>
#include "kis_assert.h"
#include <QRegularExpression>
#include <QSettings>

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif

using namespace KisOpenGLPrivate;

namespace
{
    // config option, set manually by main()
    bool g_isDebugSynchronous = false;

    bool g_sanityDefaultFormatIsSet = false;

    boost::optional<KisOpenGLModeProber::Result> openGLCheckResult;

    bool g_needsFenceWorkaround = false;
    bool g_needsPixmapCacheWorkaround = false;

    QString g_surfaceFormatDetectionLog;
    QString g_debugText("OpenGL Info\n  **OpenGL not initialized**");

    QVector<KLocalizedString> g_openglWarningStrings;
    KisOpenGL::OpenGLRenderers g_supportedRenderers;
    KisOpenGL::OpenGLRenderer g_rendererPreferredByQt;

    void overrideSupportedRenderers(KisOpenGL::OpenGLRenderers supportedRenderers, KisOpenGL::OpenGLRenderer preferredByQt) {
        g_supportedRenderers = supportedRenderers;
        g_rendererPreferredByQt = preferredByQt;
    }

    void openglOnMessageLogged(const QOpenGLDebugMessage& debugMessage) {
        qDebug() << "OpenGL:" << debugMessage;
    }
}

KisOpenGLPrivate::OpenGLCheckResult::OpenGLCheckResult(QOpenGLContext &context) {
    if (!context.isValid()) {
        return;
    }

    QOpenGLFunctions *funcs = context.functions(); // funcs is ready to be used

    m_rendererString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_RENDERER)));
    m_driverVersionString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION)));
    m_glMajorVersion = context.format().majorVersion();
    m_glMinorVersion = context.format().minorVersion();
    m_supportsDeprecatedFunctions = (context.format().options() & QSurfaceFormat::DeprecatedFunctions);
    m_isOpenGLES = context.isOpenGLES();
}

void KisOpenGLPrivate::appendOpenGLWarningString(KLocalizedString warning)
{
    g_openglWarningStrings << warning;
}

void KisOpenGLPrivate::overrideOpenGLWarningString(QVector<KLocalizedString> warnings)
{
    g_openglWarningStrings = warnings;
}


void KisOpenGL::initialize()
{
    if (openGLCheckResult) return;

    KIS_SAFE_ASSERT_RECOVER_NOOP(g_sanityDefaultFormatIsSet);

    openGLCheckResult =
        KisOpenGLModeProber::instance()->probeFormat(QSurfaceFormat::defaultFormat(), false);

    g_debugText.clear();
    QDebug debugOut(&g_debugText);
    debugOut << "OpenGL Info\n";
    debugOut << "\n  Vendor: " << openGLCheckResult->vendorString();
    debugOut << "\n  Renderer: " << openGLCheckResult->rendererString();
    debugOut << "\n  Version: " << openGLCheckResult->driverVersionString();
    debugOut << "\n  Shading language: " << openGLCheckResult->shadingLanguageString();
    debugOut << "\n  Requested format: " << QSurfaceFormat::defaultFormat();
    debugOut << "\n  Current format:   " << openGLCheckResult->format();
    debugOut.nospace();
    debugOut << "\n     Version: " << openGLCheckResult->glMajorVersion() << "." << openGLCheckResult->glMinorVersion();
    debugOut.resetFormat();
    debugOut << "\n     Supports deprecated functions" << openGLCheckResult->supportsDeprecatedFunctions();
    debugOut << "\n     is OpenGL ES:" << openGLCheckResult->isOpenGLES();

    debugOut << "\n\nQPA OpenGL Detection Info";
    debugOut << "\n  supportsDesktopGL:" << bool(g_supportedRenderers & RendererDesktopGL);
#ifdef Q_OS_WIN
    debugOut << "\n  supportsAngleD3D11:" << bool(g_supportedRenderers & RendererOpenGLES);
    debugOut << "\n  isQtPreferAngle:" << bool(g_rendererPreferredByQt == RendererOpenGLES);
#else
    debugOut << "\n  supportsOpenGLES:" << bool(g_supportedRenderers & RendererOpenGLES);
    debugOut << "\n  isQtPreferOpenGLES:" << bool(g_rendererPreferredByQt == RendererOpenGLES);
#endif
    debugOut << "\n== log ==\n";
    debugOut.noquote();
    debugOut << g_surfaceFormatDetectionLog;
    debugOut.resetFormat();
    debugOut << "\n== end log ==";

    dbgOpenGL.noquote().nospace() << g_debugText;
    KisUsageLogger::write(g_debugText);

    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif

    KisConfig cfg(true);
    if ((isOnX11 && openGLCheckResult->rendererString().startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
        g_needsFenceWorkaround = true;
    }

    /**
     * NVidia + Qt's openGL don't play well together and one cannot
     * draw a pixmap on a widget more than once in one rendering cycle.
     *
     * It can be workarounded by drawing strictly via QPixmapCache and
     * only when the pixmap size in bigger than doubled size of the
     * display framebuffer. That is for 8-bit HD display, you should have
     * a cache bigger than 16 MiB. Don't ask me why. (DK)
     *
     * See bug: https://bugs.kde.org/show_bug.cgi?id=361709
     *
     * TODO: check if this workaround is still needed after merging
     *       Qt5+openGL3 branch.
     */

    if (openGLCheckResult->vendorString().toUpper().contains("NVIDIA")) {
        g_needsPixmapCacheWorkaround = true;

        const QRect screenSize = QApplication::desktop()->screenGeometry();
        const int minCacheSize = 20 * 1024;
        const int cacheSize = 2048 + 2 * 4 * screenSize.width() * screenSize.height() / 1024; //KiB

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

// XXX Temporary function to allow LoD on OpenGL3 without triggering
// all of the other 3.2 functionality, can be removed once we move to Qt5.7
bool KisOpenGL::supportsLoD()
{
    initialize();
    return openGLCheckResult->supportsLoD();
}

bool KisOpenGL::hasOpenGL3()
{
    initialize();
    return openGLCheckResult->hasOpenGL3();
}

bool KisOpenGL::hasOpenGLES()
{
    initialize();
    return openGLCheckResult->isOpenGLES();
}

bool KisOpenGL::supportsFenceSync()
{
    initialize();
    return openGLCheckResult->supportsFenceSync();
}

bool KisOpenGL::needsFenceWorkaround()
{
    initialize();
    return g_needsFenceWorkaround;
}

bool KisOpenGL::needsPixmapCacheWorkaround()
{
    initialize();
    return g_needsPixmapCacheWorkaround;
}

void KisOpenGL::testingInitializeDefaultSurfaceFormat()
{
    setDefaultSurfaceFormat(selectSurfaceFormat(KisOpenGL::RendererAuto, KisConfig::BT709_G22, false));
}

void KisOpenGL::setDebugSynchronous(bool value)
{
    g_isDebugSynchronous = value;
}

KisOpenGL::OpenGLRenderer KisOpenGL::getCurrentOpenGLRenderer()
{
    const QSurfaceFormat::RenderableType renderer = QSurfaceFormat::defaultFormat().renderableType();

    return renderer == QSurfaceFormat::OpenGLES ? RendererOpenGLES :
           renderer == QSurfaceFormat::OpenGL ? RendererDesktopGL :
           RendererAuto;
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
    } else {
        return RendererAuto;
    }
}

namespace {

QSurfaceFormat generateSurfaceFormat(QSurfaceFormat::RenderableType renderer,
                                     KisConfig::RootSurfaceFormat rootSurfaceFormat,
                                     bool debugContext)
{
    QSurfaceFormat format;
#ifdef Q_OS_OSX
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
#else
    // XXX This can be removed once we move to Qt5.7
    format.setVersion(3, 0);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setOptions(QSurfaceFormat::DeprecatedFunctions);
#endif
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);

    KisOpenGLModeProber::initSurfaceFormatFromConfig(rootSurfaceFormat, &format);

    format.setRenderableType(renderer);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // Disable vertical refresh syncing
    if (debugContext) {
        format.setOption(QSurfaceFormat::DebugContext, true);
    }

    return format;
}

bool isOpenGLRendererBlacklisted(const QString &rendererString,
                                 const QString &driverVersionString,
                                 QVector<KLocalizedString> *warningMessage)
{
    bool isBlacklisted = false;

    // Special blacklisting of OpenGL/ANGLE is tracked on:
    // https://phabricator.kde.org/T7411

    // HACK: Specifically detect for Intel driver build number
    //       See https://www.intel.com/content/www/us/en/support/articles/000005654/graphics-drivers.html
    if (rendererString.startsWith("Intel")) {
        KLocalizedString knownBadIntelWarning = ki18n("The Intel graphics driver in use is known to have issues with OpenGL.");
        KLocalizedString grossIntelWarning = ki18n(
            "Intel graphics drivers tend to have issues with OpenGL so ANGLE will be used by default. "
            "You may manually switch to OpenGL but it is not guaranteed to work properly."
        );
        QRegularExpression regex("\\b\\d{2}\\.\\d{2}\\.\\d{2}\\.(\\d{4})\\b");
        QRegularExpressionMatch match = regex.match(driverVersionString);
        if (match.hasMatch()) {
            int driverBuild = match.captured(1).toInt();
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
            } else {
                // Intel tends to randomly break OpenGL in some of their new driver
                // builds, therefore we just shouldn't use OpenGL by default to
                // reduce bug report noises.
                qDebug() << "Detected Intel driver, making ANGLE the preferred renderer";
                isBlacklisted = true;
                *warningMessage << grossIntelWarning;
            }
        }
    }

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

    bool operator()(const QSurfaceFormat &lhs, const QSurfaceFormat &rhs) const {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_preferredColorSpace != QSurfaceFormat::DefaultColorSpace);

        ORDER_BY(isPreferredColorSpace(lhs.colorSpace()),
                 isPreferredColorSpace(rhs.colorSpace()));


        if (doPreferHDR()) {
            ORDER_BY(isHDRFormat(lhs), isHDRFormat(rhs));
        } else {
            ORDER_BY(!isHDRFormat(lhs), !isHDRFormat(rhs));
        }

        if (m_preferredRendererByUser != QSurfaceFormat::DefaultRenderableType) {
            ORDER_BY(lhs.renderableType() == m_preferredRendererByUser,
                     rhs.renderableType() == m_preferredRendererByUser);
        }

        ORDER_BY(!isBlacklisted(lhs), !isBlacklisted(rhs));

        if (doPreferHDR() &&
            m_preferredRendererByHDR != QSurfaceFormat::DefaultRenderableType) {

            ORDER_BY(lhs.renderableType() == m_preferredRendererByHDR,
                     rhs.renderableType() == m_preferredRendererByHDR);

        }

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_preferredRendererByQt != QSurfaceFormat::DefaultRenderableType);

        ORDER_BY(lhs.renderableType() == m_preferredRendererByQt,
                 rhs.renderableType() == m_preferredRendererByQt);

        return false;
    }


public:
    void setPreferredColorSpace(const QSurfaceFormat::ColorSpace &preferredColorSpace) {
        m_preferredColorSpace = preferredColorSpace;
    }

    void setPreferredRendererByQt(const QSurfaceFormat::RenderableType &preferredRendererByQt) {
        m_preferredRendererByQt = preferredRendererByQt;
    }

    void setPreferredRendererByUser(const QSurfaceFormat::RenderableType &preferredRendererByUser) {
        m_preferredRendererByUser = preferredRendererByUser;
    }

    void setPreferredRendererByHDR(const QSurfaceFormat::RenderableType &preferredRendererByHDR) {
        m_preferredRendererByHDR = preferredRendererByHDR;
    }

    void setOpenGLBlacklisted(bool openGLBlacklisted) {
        m_openGLBlacklisted = openGLBlacklisted;
    }

    void setOpenGLESBlacklisted(bool openGLESBlacklisted) {
        m_openGLESBlacklisted = openGLESBlacklisted;
    }

    QSurfaceFormat::ColorSpace preferredColorSpace() const {
        return m_preferredColorSpace;
    }

    QSurfaceFormat::RenderableType preferredRendererByUser() const {
        return m_preferredRendererByUser;
    }

private:
    bool isHDRFormat(const QSurfaceFormat &f) const {
#ifdef HAVE_HDR
        return f.colorSpace() == QSurfaceFormat::bt2020PQColorSpace ||
            f.colorSpace() == QSurfaceFormat::scRGBColorSpace;
#else
        Q_UNUSED(f);
        return false;
#endif
    }

    bool isBlacklisted(const QSurfaceFormat &f) const {
        KIS_SAFE_ASSERT_RECOVER_NOOP(f.renderableType() == QSurfaceFormat::OpenGL ||
                                     f.renderableType() == QSurfaceFormat::OpenGLES);

        return (f.renderableType() == QSurfaceFormat::OpenGL && m_openGLBlacklisted) ||
            (f.renderableType() == QSurfaceFormat::OpenGLES && m_openGLESBlacklisted);
    }

    bool doPreferHDR() const {
#ifdef HAVE_HDR
        return m_preferredColorSpace == QSurfaceFormat::bt2020PQColorSpace ||
            m_preferredColorSpace == QSurfaceFormat::scRGBColorSpace;
#else
        return false;
#endif
    }

    bool isPreferredColorSpace(const QSurfaceFormat::ColorSpace cs) const {
        return KisOpenGLModeProber::fuzzyCompareColorSpaces(m_preferredColorSpace, cs);
        return false;
    }

private:
    QSurfaceFormat::ColorSpace m_preferredColorSpace = QSurfaceFormat::DefaultColorSpace;
    QSurfaceFormat::RenderableType m_preferredRendererByQt = QSurfaceFormat::OpenGL;
    QSurfaceFormat::RenderableType m_preferredRendererByUser = QSurfaceFormat::DefaultRenderableType;
    QSurfaceFormat::RenderableType m_preferredRendererByHDR = QSurfaceFormat::DefaultRenderableType;
    bool m_openGLBlacklisted = false;
    bool m_openGLESBlacklisted = false;
};

struct DetectionDebug : public QDebug
{
    DetectionDebug(QString *string)
        : QDebug(string),
          m_string(string),
          m_originalSize(string->size())
    {}
    ~DetectionDebug() { dbgOpenGL << m_string->right(m_string->size() - m_originalSize); *this << endl; }

    QString *m_string;
    int m_originalSize;
};
}

#define dbgDetection() DetectionDebug(&g_surfaceFormatDetectionLog)

QSurfaceFormat KisOpenGL::selectSurfaceFormat(KisOpenGL::OpenGLRenderer preferredRenderer,
                                              KisConfig::RootSurfaceFormat preferredRootSurfaceFormat,
                                              bool enableDebug)
{
    QVector<KLocalizedString> warningMessages;

    using Info = boost::optional<KisOpenGLModeProber::Result>;

    QVector<QSurfaceFormat::RenderableType> renderers({QSurfaceFormat::OpenGLES, QSurfaceFormat::OpenGL});

#ifdef HAVE_HDR
    QVector<KisConfig::RootSurfaceFormat> formatSymbols({KisConfig::BT709_G22, KisConfig::BT709_G10, KisConfig::BT2020_PQ});
#else
    QVector<KisConfig::RootSurfaceFormat> formatSymbols({KisConfig::BT709_G22});
#endif

    QVector<QSurfaceFormat> preferredFormats;
    Q_FOREACH (const QSurfaceFormat::RenderableType renderer, renderers) {
        Q_FOREACH (const KisConfig::RootSurfaceFormat formatSymbol, formatSymbols) {
            preferredFormats << generateSurfaceFormat(renderer, formatSymbol, enableDebug);
        }
    }

    QSurfaceFormat defaultFormat = generateSurfaceFormat(QSurfaceFormat::DefaultRenderableType,
                                                         KisConfig::BT709_G22, false);
    Info info = KisOpenGLModeProber::instance()->probeFormat(defaultFormat);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(info, QSurfaceFormat());

    FormatPositionLess compareOp;
    compareOp.setPreferredRendererByQt(info->isOpenGLES() ? QSurfaceFormat::OpenGLES : QSurfaceFormat::OpenGL);

#ifdef HAVE_HDR
    compareOp.setPreferredColorSpace(
        preferredRootSurfaceFormat == KisConfig::BT709_G22 ? QSurfaceFormat::sRGBColorSpace :
        preferredRootSurfaceFormat == KisConfig::BT709_G10 ? QSurfaceFormat::scRGBColorSpace :
        QSurfaceFormat::bt2020PQColorSpace);
#else
    Q_UNUSED(preferredRootSurfaceFormat);
    compareOp.setPreferredColorSpace(QSurfaceFormat::sRGBColorSpace);
#endif

#ifdef Q_OS_WIN
    compareOp.setPreferredRendererByHDR(QSurfaceFormat::OpenGLES);
#endif
    compareOp.setPreferredRendererByUser(preferredRenderer == KisOpenGL::RendererDesktopGL ? QSurfaceFormat::OpenGL :
                                         preferredRenderer == KisOpenGL::RendererOpenGLES ? QSurfaceFormat::OpenGLES :
                                         QSurfaceFormat::DefaultRenderableType);
    compareOp.setOpenGLESBlacklisted(false); // We cannot blacklist ES drivers atm

    OpenGLRenderers supportedRenderers = RendererNone;
    OpenGLRenderer preferredByQt = info->isOpenGLES() ? RendererOpenGLES : RendererDesktopGL;

    if (!info->isOpenGLES()) {
        compareOp.setOpenGLBlacklisted(isOpenGLRendererBlacklisted(info->rendererString(),
                                                                   info->driverVersionString(),
                                                                   &warningMessages));

        supportedRenderers |= RendererDesktopGL;

        info = KisOpenGLModeProber::instance()->
            probeFormat(generateSurfaceFormat(QSurfaceFormat::OpenGLES,
                                              KisConfig::BT709_G22, false));
        if (info) {
            supportedRenderers |= RendererOpenGLES;
        }
    } else {
        supportedRenderers |= RendererOpenGLES;

        info = KisOpenGLModeProber::instance()->
            probeFormat(generateSurfaceFormat(QSurfaceFormat::OpenGL,
                                              KisConfig::BT709_G22, false));

        if (!info || info->isOpenGLES()) {
            compareOp.setOpenGLBlacklisted(true);
        } else {
            compareOp.setOpenGLBlacklisted(isOpenGLRendererBlacklisted(info->rendererString(),
                                                                       info->driverVersionString(),
                                                                       &warningMessages));

            supportedRenderers |= RendererDesktopGL;
        }
    }

    std::stable_sort(preferredFormats.begin(), preferredFormats.end(), compareOp);

    dbgDetection() << "Supported renderers:" << supportedRenderers;

    dbgDetection() << "Surface format preference list:";
    Q_FOREACH (const QSurfaceFormat &format, preferredFormats) {
        dbgDetection() << "*" << format;
        dbgDetection() << "   " << format.renderableType();
    }

    QSurfaceFormat resultFormat = defaultFormat;

    Q_FOREACH (const QSurfaceFormat &format, preferredFormats) {
        dbgDetection() <<"Probing format..." << format.colorSpace() << format.renderableType();

        Info info = KisOpenGLModeProber::instance()->probeFormat(format);

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

            dbgDetection() << "Found format:" << format;
            dbgDetection() << "   " << format.renderableType();

            resultFormat = format;
            break;
        }
    }

    {
        const bool colorSpaceIsCorrect =
            KisOpenGLModeProber::fuzzyCompareColorSpaces(compareOp.preferredColorSpace(),
                                                         resultFormat.colorSpace());

        const bool rendererIsCorrect =
            compareOp.preferredRendererByUser() == QSurfaceFormat::DefaultRenderableType ||
            compareOp.preferredRendererByUser() == resultFormat.renderableType();

        if (!rendererIsCorrect && colorSpaceIsCorrect) {
            warningMessages << ki18n("Preferred renderer doesn't support requested surface format. Another renderer has been selected.");
        } else if (!colorSpaceIsCorrect) {
            warningMessages << ki18n("Preferred output format is not supported by available renderers");
        }

    }

    overrideSupportedRenderers(supportedRenderers, preferredByQt);
    overrideOpenGLWarningString(warningMessages);

    return resultFormat;
}

void KisOpenGL::setDefaultSurfaceFormat(const QSurfaceFormat &format)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!g_sanityDefaultFormatIsSet);

    g_sanityDefaultFormatIsSet = true;
    QSurfaceFormat::setDefaultFormat(format);
}

bool KisOpenGL::hasOpenGL()
{
    return openGLCheckResult->isSupportedVersion();
}
