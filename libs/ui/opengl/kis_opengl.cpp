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
#include <QOpenGLFunctions>

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>

#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QWindow>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_config.h>

namespace
{
    bool initialized = false;
    bool NeedsFenceWorkaround = false;
    bool NeedsPixmapCacheWorkaround = false;
    int glMajorVersion = 0;
    int glMinorVersion = 0;
    bool supportsDeprecatedFunctions = false;

    QString Renderer;
}

void KisOpenGL::initialize()
{
    if (initialized) return;

    setDefaultFormat();

    // we need a QSurface active to get our GL functions from the context
    QWindow  surface;
    surface.setSurfaceType( QSurface::OpenGLSurface );
    surface.create();

    QOpenGLContext context;
    context.create();
    if (!context.isValid()) return;

    context.makeCurrent( &surface );

    QOpenGLFunctions  *funcs = context.functions();
    funcs->initializeOpenGLFunctions();

    qDebug() << "OpenGL Info";
    qDebug() << "  Vendor: " << reinterpret_cast<const char *>(funcs->glGetString(GL_VENDOR));
    qDebug() << "  Renderer: " << reinterpret_cast<const char *>(funcs->glGetString(GL_RENDERER));
    qDebug() << "  Version: " << reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION));
    qDebug() << "  Shading language: " << reinterpret_cast<const char *>(funcs->glGetString(GL_SHADING_LANGUAGE_VERSION));
    qDebug() << "  Requested format: " << QSurfaceFormat::defaultFormat();
    qDebug() << "  Current format:   " << context.format();

    glMajorVersion = context.format().majorVersion();
    glMinorVersion = context.format().minorVersion();
    supportsDeprecatedFunctions = (context.format().options() & QSurfaceFormat::DeprecatedFunctions);

    qDebug() << "     Version:" << glMajorVersion << "." << glMinorVersion;
    qDebug() << "     Supports deprecated functions" << supportsDeprecatedFunctions;

    initialized = true;
}

void KisOpenGL::initializeContext(QOpenGLContext *ctx)
{
    initialize();

    dbgUI << "OpenGL: Opening new context";

    // Double check we were given the version we requested
    QSurfaceFormat format = ctx->format();
    QOpenGLFunctions *f = ctx->functions();
    f->initializeOpenGLFunctions();

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif
    Renderer = QString((const char*)f->glGetString(GL_RENDERER));

    QFile log(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/krita-opengl.txt");
    log.open(QFile::WriteOnly);
    QString vendor((const char*)f->glGetString(GL_VENDOR));
    log.write(vendor.toLatin1());
    log.write(", ");
    log.write(Renderer.toLatin1());
    log.write(", ");
    QString version((const char*)f->glGetString(GL_VERSION));
    log.write(version.toLatin1());
    log.close();

    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif
    KisConfig cfg;
    if ((isOnX11 && Renderer.startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
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


    /**
     * Warn about Intel's broken video drivers
     */
#if defined Q_OS_WIN
    if (cfg.useOpenGL() && Renderer.startsWith("Intel") && !cfg.readEntry("WarnedAboutIntel", false)) {
        QMessageBox::information(0,
                                 i18nc("@title:window", "Krita: Warning"),
                                 i18n("You have an Intel(R) HD Graphics video adapter.\n"
                                      "If you experience problems like a crash, a black or blank screen,"
                                      "please update your display driver to the latest version.\n\n"
                                      "If Krita crashes, it will disable OpenGL rendering. Please restart Krita in that case.\n After updating your drivers you can re-enable OpenGL in Krita's Settings.\n"));
        cfg.writeEntry("WarnedAboutIntel", true);
    }
#endif

}

// XXX Temporary function to allow LoD on OpenGL3 without triggering
// all of the other 3.2 functionality, can be removed once we move to Qt5.7
bool KisOpenGL::supportsLoD()
{
    initialize();
    return (glMajorVersion * 100 + glMinorVersion) >= 300;
}

bool KisOpenGL::hasOpenGL3()
{
    initialize();
    return (glMajorVersion * 100 + glMinorVersion) >= 302;
}

bool KisOpenGL::supportsFenceSync()
{
    initialize();
    return glMajorVersion >= 3;
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

void KisOpenGL::setDefaultFormat()
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
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

bool KisOpenGL::hasOpenGL()
{
    return ((glMajorVersion * 100 + glMinorVersion) >= 201);
    //return (glMajorVersion >= 3 && supportsDeprecatedFunctions);
}
