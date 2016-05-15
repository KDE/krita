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
    bool NeedsFenceWorkaround = false;
    int glVersion = 0;
    QString Renderer;
}

class TestWindow : public QWindow {
public:
    TestWindow(QScreen *screen = 0)
        : QWindow(screen)
    {
        QOpenGLContext *context = 0;
        QSurfaceFormat format;

        // 3.2 Compatibility
        format.setMajorVersion( 3 );
        format.setMinorVersion( 2 );
        format.setProfile( QSurfaceFormat::CoreProfile );
        setFormat( format );
        // Create an OpenGL context
        context = new QOpenGLContext;
        context->setFormat( format );
        context->create();
        context->makeCurrent(this);
        QOpenGLFunctions_3_2_Core *f0 = context->versionFunctions<QOpenGLFunctions_3_2_Core>();
        version32Core = f0;
        delete context;

        // 3.2 Core
        format.setProfile( QSurfaceFormat::CompatibilityProfile );
        setFormat( format );
        // Create an OpenGL context
        context = new QOpenGLContext;
        context->setFormat( format );
        context->create();
        context->makeCurrent(this);
        QOpenGLFunctions_3_2_Compatibility *f1 = context->versionFunctions<QOpenGLFunctions_3_2_Compatibility>();
        version32Compatibility = f1;
        delete context;

        // 2.1
        format.setMajorVersion( 3 );
        format.setMinorVersion( 2 );
        setFormat( format );
        // Create an OpenGL context
        context = new QOpenGLContext;
        context->setFormat( format );
        context->create();
        context->makeCurrent(this);
        QOpenGLFunctions_2_1 *f2 = context->versionFunctions<QOpenGLFunctions_2_1>();
        version32Core = f2;
        delete context;

    }

    bool version32Core;
    bool version32Compatibility;
    bool version21;
};


void KisOpenGL::initialize()
{
    {
        TestWindow w;
        qDebug() << "3.2 core" << w.version32Core << "3.2 compatibility" << w.version32Compatibility << "2.1" << w.version21;
    }

    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setOptions(QSurfaceFormat::DeprecatedFunctions);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 2);
    // if (cfg.disableDoubleBuffering()) {
    if (false) {
        format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    }
    else {
        format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    }
    format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

int KisOpenGL::initializeContext(QOpenGLContext* s) {
    KisConfig cfg;
    dbgUI << "OpenGL: Opening new context";

    // Double check we were given the version we requested
    QSurfaceFormat format = s->format();
    glVersion = 100 * format.majorVersion() + format.minorVersion();
    qDebug() << "glVersion";
    QOpenGLFunctions *f = s->functions();

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif
    Renderer = QString((const char*)f->glGetString(GL_RENDERER));

    QFile log(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/krita-opengl.txt");
    dbgUI << "Writing OpenGL log to" << log.fileName();
    log.open(QFile::WriteOnly);
    QString vendor((const char*)f->glGetString(GL_VENDOR));
    log.write(vendor.toLatin1());
    log.write(", ");
    log.write(Renderer.toLatin1());
    log.write(", ");
    QString version((const char*)f->glGetString(GL_VERSION));
    log.write(version.toLatin1());

    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif

    if ((isOnX11 && Renderer.startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
        NeedsFenceWorkaround = true;
    }

    return glVersion;
}

bool KisOpenGL::supportsFenceSync()
{
    glVersion = 100 * QSurfaceFormat::defaultFormat().majorVersion() + QSurfaceFormat::defaultFormat().minorVersion();
    dbgOpenGL << "GL Version:" << glVersion << QSurfaceFormat::defaultFormat().swapInterval() << QSurfaceFormat::defaultFormat().swapBehavior();

    return glVersion >= 302;
}

bool KisOpenGL::needsFenceWorkaround()
{
    return NeedsFenceWorkaround;
}

QString KisOpenGL::renderer()
{
    return Renderer;
}

bool KisOpenGL::hasOpenGL()
{
    glVersion = 100 * QSurfaceFormat::defaultFormat().majorVersion() + QSurfaceFormat::defaultFormat().minorVersion();
    qDebug() << "GL Version:" << glVersion << QSurfaceFormat::defaultFormat().swapInterval() << QSurfaceFormat::defaultFormat().swapBehavior();

    return glVersion >= 302;
}
