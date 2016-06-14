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
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QWindow>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_2_1>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_config.h>

namespace
{
    bool initialized = false;
    bool NeedsFenceWorkaround = false;
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
}

bool KisOpenGL::supportsGLSL13()
{
    initialize();
    return glMajorVersion >= 3 && supportsDeprecatedFunctions;
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

void KisOpenGL::setDefaultFormat()
{
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 2);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    KisConfig cfg;
    if (cfg.disableVSync()) {
        format.setSwapInterval(0); // Disable vertical refresh syncing
    }
    QSurfaceFormat::setDefaultFormat(format);
}

bool KisOpenGL::hasOpenGL()
{
    return ((glMajorVersion * 100 + glMinorVersion) >= 201);
    //return (glMajorVersion >= 3 && supportsDeprecatedFunctions);
}
