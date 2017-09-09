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

#include "opengl/kis_opengl.h"

#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>

#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QStringList>
#include <QWindow>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_config.h>
#include <KisLoggingManager.h>

#include <boost/optional.hpp>

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif

namespace
{
    bool defaultFormatIsSet = false;
    bool isDebugEnabled = false;
    bool isDebugSynchronous = false;

    struct OpenGLCheckResult {
        // bool contextValid = false;
        int glMajorVersion = 0;
        int glMinorVersion = 0;
        bool supportsDeprecatedFunctions = false;
        bool isOpenGLES = false;
        QString rendererString;

        OpenGLCheckResult(QOpenGLContext &context) {
            if (!context.isValid()) {
                return;
            }

            QOpenGLFunctions *funcs = context.functions(); // funcs is ready to be used

            rendererString = QString((const char*)funcs->glGetString(GL_RENDERER));
            glMajorVersion = context.format().majorVersion();
            glMinorVersion = context.format().minorVersion();
            supportsDeprecatedFunctions = (context.format().options() & QSurfaceFormat::DeprecatedFunctions);
            isOpenGLES = context.isOpenGLES();
        }

        bool isSupportedVersion() const {
            return
#ifdef Q_OS_OSX
                    ((glMajorVersion * 100 + glMinorVersion) >= 302)
#else
                    (glMajorVersion >= 3 && (supportsDeprecatedFunctions || isOpenGLES)) ||
                    ((glMajorVersion * 100 + glMinorVersion) == 201)
#endif
                    ;
        }

        bool supportsLoD() const {
            return (glMajorVersion * 100 + glMinorVersion) >= 300;
        }

        bool hasOpenGL3() const {
            return (glMajorVersion * 100 + glMinorVersion) >= 302;
        }

        bool supportsFenceSync() const {
            return glMajorVersion >= 3;
        }
    };
    boost::optional<OpenGLCheckResult> openGLCheckResult;

    bool NeedsFenceWorkaround = false;
    bool NeedsPixmapCacheWorkaround = false;

    QString debugText("OpenGL Info\n  **OpenGL not initialized**");

    void openglOnMessageLogged(const QOpenGLDebugMessage& debugMessage) {
        qDebug() << "OpenGL:" << debugMessage;
    }
}

#ifdef Q_OS_WIN
namespace
{
    struct WinOpenGLCheckResult {
        bool openGLValid;
        bool usingAngle;
    };

    QStringList qpaDetectionLog;
    WinOpenGLCheckResult qpaDetectionResult = {};

    WinOpenGLCheckResult checkQpaOpenGLStatus() {
        WinOpenGLCheckResult retval;
        retval.openGLValid = false;
        retval.usingAngle = false;

        QOpenGLContext *context = QOpenGLContext::globalShareContext();
        if (!context) {
            qDebug() << "Global shared OpenGL context is null while checking Qt's OpenGL detection";
            return retval;
        }
        if (!context->isValid()) {
            qDebug() << "Global shared OpenGL context is not valid while checking Qt's OpenGL detection";
            return retval;
        }
        retval.openGLValid = true;
        retval.usingAngle = context->isOpenGLES();
        return retval;
    }
} // namespace

/**
 * This function probes the Qt Platform Abstraction (QPA) for OpenGL diagnostics
 * information. The code works under the assumption that the bundled Qt is built
 * with `-opengl dynamic` and includes support for ANGLE.
 *
 * This function is written for Qt 5.9.1. On other versions it might not work
 * as well.
 */
void KisOpenGL::probeWindowsQpaOpenGL(int argc, char **argv)
{
    // Clear env var to prevent affecting tests
    QByteArray userQtOpenGLEnv = qgetenv("QT_OPENGL");
    qunsetenv("QT_OPENGL");

    qDebug() << "Probing Qt OpenGL detection:";
    {
        KisLoggingManager::ScopedLogCapturer logCapturer(
            "qt.qpa.gl",
            [&qpaDetectionLog](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
                Q_UNUSED(type)
                Q_UNUSED(context)
                qpaDetectionLog.append(msg);
            }
        );
        {
            QGuiApplication app(argc, argv);
            qpaDetectionResult = checkQpaOpenGLStatus();
        }
    }
    qDebug() << "Done probing Qt OpenGL detection";

    // Restore env var
    qputenv("QT_OPENGL", userQtOpenGLEnv);
}
#endif

void KisOpenGL::initialize()
{
    if (openGLCheckResult) return;

    KIS_SAFE_ASSERT_RECOVER(defaultFormatIsSet) {
        qWarning() << "Default OpenGL format was not set before calling KisOpenGL::initialize. This might be a BUG!";
        setDefaultFormat();
    }

    // we need a QSurface active to get our GL functions from the context
    QWindow  surface;
    surface.setSurfaceType( QSurface::OpenGLSurface );
    surface.create();

    QOpenGLContext context;
    context.create();
    if (!context.isValid()) return;

    context.makeCurrent( &surface );

    QOpenGLFunctions  *funcs = context.functions();
    openGLCheckResult = OpenGLCheckResult(context);

    /**
     * Warn about Intel's broken video drivers
     */
#if defined Q_OS_WIN
    KisConfig cfg;
    if (cfg.useOpenGL() && openGLCheckResult->rendererString.startsWith("Intel") && !cfg.readEntry("WarnedAboutIntel", false)) {
        QMessageBox::information(0,
                                 i18nc("@title:window", "Krita: Warning"),
                                 i18n("You have an Intel(R) HD Graphics video adapter.\n"
                                      "If you experience problems like a crash, a black or blank screen,"
                                      "please update your display driver to the latest version.\n\n"
                                      "If Krita crashes, it will disable OpenGL rendering. Please restart Krita in that case.\n After updating your drivers you can re-enable OpenGL in Krita's Settings.\n"));
        cfg.writeEntry("WarnedAboutIntel", true);
    }
#endif


    debugText.clear();
    QDebug debugOut(&debugText);
    debugOut << "OpenGL Info";
    debugOut << "\n  Vendor: " << reinterpret_cast<const char *>(funcs->glGetString(GL_VENDOR));
    debugOut << "\n  Renderer: " << openGLCheckResult->rendererString;
    debugOut << "\n  Version: " << reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION));
    debugOut << "\n  Shading language: " << reinterpret_cast<const char *>(funcs->glGetString(GL_SHADING_LANGUAGE_VERSION));
    debugOut << "\n  Requested format: " << QSurfaceFormat::defaultFormat();
    debugOut << "\n  Current format:   " << context.format();
    debugOut.nospace();
    debugOut << "\n     Version: " << openGLCheckResult->glMajorVersion << "." << openGLCheckResult->glMinorVersion;
    debugOut.resetFormat();
    debugOut << "\n     Supports deprecated functions" << openGLCheckResult->supportsDeprecatedFunctions;
    debugOut << "\n     is OpenGL ES:" << openGLCheckResult->isOpenGLES;
#ifdef Q_OS_WIN
    debugOut << "\n\nQPA OpenGL Detection Info";
    debugOut << "\n  openGLValid:" << qpaDetectionResult.openGLValid;
    debugOut << "\n  usingAngle:" << qpaDetectionResult.usingAngle;
    debugOut << "\n== log ==\n";
    debugOut.noquote();
    debugOut << qpaDetectionLog.join('\n');
    debugOut.resetFormat();
    debugOut << "\n== end log ==";
#endif

    qDebug().noquote() << debugText;

}

void KisOpenGL::initializeContext(QOpenGLContext *ctx)
{
    KisConfig cfg;
    initialize();

    dbgUI << "OpenGL: Opening new context";
    if (isDebugEnabled) {
        // Passing ctx for ownership management only, not specifying context.
        // QOpenGLDebugLogger only function on the current active context.
        // FIXME: Do we need to make sure ctx is the active context?
        QOpenGLDebugLogger* openglLogger = new QOpenGLDebugLogger(ctx);
        if (openglLogger->initialize()) {
            qDebug() << "QOpenGLDebugLogger is initialized. Check whether you get a message below.";
            QObject::connect(openglLogger, &QOpenGLDebugLogger::messageLogged, &openglOnMessageLogged);
            openglLogger->startLogging(isDebugSynchronous ? QOpenGLDebugLogger::SynchronousLogging : QOpenGLDebugLogger::AsynchronousLogging);
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

    QFile log(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/krita-opengl.txt");
    log.open(QFile::WriteOnly);
    QString vendor((const char*)f->glGetString(GL_VENDOR));
    log.write(vendor.toLatin1());
    log.write(", ");
    log.write(openGLCheckResult->rendererString.toLatin1());
    log.write(", ");
    QString version((const char*)f->glGetString(GL_VERSION));
    log.write(version.toLatin1());
    log.close();

    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif

    if ((isOnX11 && openGLCheckResult->rendererString.startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
        NeedsFenceWorkaround = true;
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

    if (vendor.toUpper().contains("NVIDIA")) {
        NeedsPixmapCacheWorkaround = true;

        const QRect screenSize = QApplication::desktop()->screenGeometry();
        const int minCacheSize = 20 * 1024;
        const int cacheSize = 2048 + 2 * 4 * screenSize.width() * screenSize.height() / 1024; //KiB

        QPixmapCache::setCacheLimit(qMax(minCacheSize, cacheSize));
    }


}

const QString &KisOpenGL::getDebugText()
{
    initialize();
    return debugText;
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
    return openGLCheckResult->isOpenGLES;
}

bool KisOpenGL::supportsFenceSync()
{
    initialize();
    return openGLCheckResult->supportsFenceSync();
}

bool KisOpenGL::needsFenceWorkaround()
{
    initialize();
    return NeedsFenceWorkaround;
}

bool KisOpenGL::needsPixmapCacheWorkaround()
{
    initialize();
    return NeedsPixmapCacheWorkaround;
}

void KisOpenGL::setDefaultFormat(bool enableDebug, bool debugSynchronous)
{
    if (defaultFormatIsSet) {
        return;
    }
    defaultFormatIsSet = true;
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
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // Disable vertical refresh syncing
    isDebugEnabled = enableDebug;
    if (enableDebug) {
        format.setOption(QSurfaceFormat::DebugContext, true);
        isDebugSynchronous = debugSynchronous;
        qDebug() << "QOpenGLDebugLogger will be enabled, synchronous:" << debugSynchronous;
    }
    QSurfaceFormat::setDefaultFormat(format);
}

bool KisOpenGL::hasOpenGL()
{
    return openGLCheckResult->isSupportedVersion();
}
