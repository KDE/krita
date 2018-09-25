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

#include <boost/optional.hpp>

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif

using namespace KisOpenGLPrivate;

namespace
{
    bool defaultFormatIsSet = false;
    bool isDebugEnabled = false;
    bool isDebugSynchronous = false;

    boost::optional<OpenGLCheckResult> openGLCheckResult;

    bool NeedsFenceWorkaround = false;
    bool NeedsPixmapCacheWorkaround = false;

    QString debugText("OpenGL Info\n  **OpenGL not initialized**");

    QVector<KLocalizedString> openglWarningStrings;

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
    openglWarningStrings << warning;
}

bool KisOpenGLPrivate::isDefaultFormatSet() {
    return defaultFormatIsSet;
}

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
    if (!context.create()) {
        qDebug() << "OpenGL context cannot be created";
        return;
    }
    if (!context.isValid()) {
        qDebug() << "OpenGL context is not valid";
        return;
    }

    if (!context.makeCurrent(&surface)) {
        qDebug() << "OpenGL context cannot be made current";
        return;
    }

    QOpenGLFunctions  *funcs = context.functions();
    openGLCheckResult = OpenGLCheckResult(context);
    debugText.clear();
    QDebug debugOut(&debugText);
    debugOut << "OpenGL Info";
    debugOut << "\n  Vendor: " << reinterpret_cast<const char *>(funcs->glGetString(GL_VENDOR));
    debugOut << "\n  Renderer: " << openGLCheckResult->rendererString();
    debugOut << "\n  Version: " << openGLCheckResult->driverVersionString();
    debugOut << "\n  Shading language: " << reinterpret_cast<const char *>(funcs->glGetString(GL_SHADING_LANGUAGE_VERSION));
    debugOut << "\n  Requested format: " << QSurfaceFormat::defaultFormat();
    debugOut << "\n  Current format:   " << context.format();
    debugOut.nospace();
    debugOut << "\n     Version: " << openGLCheckResult->glMajorVersion() << "." << openGLCheckResult->glMinorVersion();
    debugOut.resetFormat();
    debugOut << "\n     Supports deprecated functions" << openGLCheckResult->supportsDeprecatedFunctions();
    debugOut << "\n     is OpenGL ES:" << openGLCheckResult->isOpenGLES();
    appendPlatformOpenGLDebugText(debugOut);

    dbgOpenGL.noquote() << debugText;

}

void KisOpenGL::initializeContext(QOpenGLContext *ctx)
{
    KisConfig cfg(true);
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

    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif

    if ((isOnX11 && openGLCheckResult->rendererString().startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
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

QStringList KisOpenGL::getOpenGLWarnings() {
    QStringList strings;
    Q_FOREACH (const KLocalizedString &item, openglWarningStrings) {
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
