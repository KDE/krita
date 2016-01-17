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

#ifdef HAVE_OPENGL
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#endif

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QMessageBox>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_config.h>

namespace
{
#ifdef HAVE_OPENGL
#endif
    bool NeedsFenceWorkaround = false;
    int glVersion = 0;
}


void KisOpenGL::initialize()
{
#ifdef HAVE_OPENGL
    dbgUI << "OpenGL: initializing";

    KisConfig cfg;

    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
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

#endif
}

int KisOpenGL::initializeContext(QOpenGLContext* s) {
#ifdef HAVE_OPENGL
    KisConfig cfg;
    dbgUI << "OpenGL: Opening new context";

    // Double check we were given the version we requested
    QSurfaceFormat format = s->format();
    glVersion = 100 * format.majorVersion() + format.minorVersion();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();


    /**
     * Warn about Intel's broken video drivers
     */
#if defined HAVE_OPENGL && defined Q_OS_WIN
#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif
    QString renderer = QString((const char*)f->glGetString(GL_RENDERER));
    if (cfg.useOpenGL() && renderer.startsWith("Intel") && !cfg.readEntry("WarnedAboutIntel", false)) {
        QMessageBox::information(0,
                                 i18nc("@title:window", "Krita: Warning"),
                                 i18n("You have an Intel(R) HD Graphics video adapter.\n"
                                      "If you experience problems like a black or blank screen,"
                                      "please update your display driver to the latest version.\n\n"
                                      "You can also disable OpenGL rendering in Krita's Settings.\n"));
        cfg.writeEntry("WarnedAboutIntel", true);
    }
#endif


    QFile log(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/krita-opengl.txt");
    dbgUI << "Writing OpenGL log to" << log.fileName();
    log.open(QFile::WriteOnly);
    QString vendor((const char*)f->glGetString(GL_VENDOR));
    log.write(vendor.toLatin1());
    log.write(", ");
    QString renderer((const char*)f->glGetString(GL_RENDERER));
    log.write(renderer.toLatin1());
    log.write(", ");
    QString version((const char*)f->glGetString(GL_VERSION));
    log.write(version.toLatin1());

    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif

    if ((isOnX11 && renderer.startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
        NeedsFenceWorkaround = true;
    }
#else
    Q_UNUSED(s);
    NeedsFenceWorkaround = false;
#endif
    return glVersion;
}

bool KisOpenGL::supportsFenceSync()
{
    // return glVersion > 302;
    return true;
}

bool KisOpenGL::needsFenceWorkaround()
{
    return NeedsFenceWorkaround;
}

bool KisOpenGL::hasOpenGL()
{
    // QT5TODO: figure out runtime whether we have opengl...
    return true;
}
