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

#include <QGLContext>
#include <QGLWidget>
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QMessageBox>

#include <klocale.h>

#include <kis_debug.h>
#include <kis_config.h>

namespace
{
    QGLWidget *SharedContextWidget = 0;
    bool NeedsFenceWorkaround = false;
}

void KisOpenGL::createContext()
{
    Q_ASSERT(SharedContextWidget == 0);

    dbgUI << "OpenGL: Creating shared context widget";

    if (!QGLFormat::hasOpenGL()) {
        qWarning() << "OpenGL is not available at all, falling back to CPU Canvas";
        return;
    }

    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_None)
        qDebug() << "no OpenGL is present or if no OpenGL context is current.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_1)
        qDebug() << "OpenGL version 1.1 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_2)
        qDebug() << "OpenGL version 1.2 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_3)
        qDebug() << "OpenGL version 1.3 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_4)
        qDebug() << "OpenGL version 1.4 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_5)
        qDebug() << "OpenGL version 1.5 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0)
        qDebug() << "OpenGL version 2.0 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_1)
        qDebug() << "OpenGL version 2.1 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_3_0)
        qDebug() << "OpenGL version 3.0 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_3_1)
        qDebug() << "OpenGL version 3.1 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_3_2)
        qDebug() << "OpenGL version 3.2 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_3_3)
        qDebug() << "OpenGL version 3.3 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_4_0)
        qDebug() << "OpenGL version 4.0 or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_CommonLite_Version_1_0)
        qDebug() << "OpenGL ES version 1.0 Common Lite or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Common_Version_1_0)
        qDebug() << "OpenGL ES version 1.0 Common or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_CommonLite_Version_1_1)
        qDebug() << "OpenGL ES version 1.1 Common Lite or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Common_Version_1_1)
        qDebug() << "OpenGL ES version 1.1 Common or higher is present.";
    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Version_2_0)
        qDebug() << "OpenGL ES version 2.0 or higher is present.";

    if (!((QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0) ||
          (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Version_2_0))) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Cannot use OpenGL: Krita needs at least OpenGL 2.0 or ES 2.0."));
        KisConfig cfg;
        cfg.setUseOpenGL(false);
        return;
    }

    QGLFormat format(QGL::SampleBuffers);

/* Profiles are broken in Qt 4.x: http://qt-project.org/forums/viewthread/4856
#if QT_VERSION >= 0x040700
    format.setVersion(3, 0);
    format.setProfile(QGLFormat::CoreProfile);
    if (format.profile() == 0)
        qDebug() << "Using No Profile";
    if (format.profile() == 1)
        qDebug() << "Using the core profile";
    if (format.profile() == 2)
        qDebug() << "Using the compatibility profile";
#endif
*/

    KisConfig cfg;
    if (cfg.disableDoubleBuffering()) {
        format.setDoubleBuffer(false);
    }

    SharedContextWidget = new QGLWidget(format);
    SharedContextWidget->setObjectName("Krita OpenGL Shared Context Widget");
    SharedContextWidget->makeCurrent();

#ifdef HAVE_GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        dbgUI << "glewInit error:" << (const char *)glewGetErrorString(err);
    } else {
        dbgUI << "Status: Using GLEW" << (const char *)glewGetString(GLEW_VERSION);
    }
#endif

#ifdef Q_OS_WIN
    {
        QFile f(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/krita-opengl.txt");
        f.open(QFile::WriteOnly);
        QString vendor((const char*)glGetString(GL_VENDOR));
        f.write(vendor.toLatin1());
        f.write(", ");
        QString renderer((const char*)glGetString(GL_RENDERER));
        f.write(renderer.toLatin1());
        f.write(", ");
        QString version((const char*)glGetString(GL_VERSION));
        f.write(version.toLatin1());
    }
#endif

    // Check if we have a bugged driver that needs fence workaround
    QString renderer((const char*)glGetString(GL_RENDERER));
    if (renderer.startsWith("AMD")) {
        NeedsFenceWorkaround = true;
    }
}

void KisOpenGL::initialMakeContextCurrent()
{
    sharedContextWidget()->makeCurrent();
}

void KisOpenGL::makeContextCurrent()
{
#ifdef Q_OS_WIN
    sharedContextWidget()->makeCurrent();
#endif
}

QGLWidget *KisOpenGL::sharedContextWidget()
{
    if (SharedContextWidget == 0) {
        createContext();
    }
    return SharedContextWidget;
}

void KisOpenGL::printError(const char *file, int line)
{
    GLenum glErr = glGetError();

    while (glErr != GL_NO_ERROR) {

        dbgUI << "glError:" << (const char *)gluErrorString(glErr);

        if (file != 0) {
            if (line != -1) {
                dbgUI << " at" << file << " line" << line;
            } else {
                dbgUI << " in" << file;
            }
        }

        glErr = glGetError();
    }
}

void KisOpenGL::clearError()
{
    while (glGetError() != GL_NO_ERROR) {
    }
}

bool KisOpenGL::supportsGLSL13()
{
    return QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_3_0;
}

bool KisOpenGL::needsFenceWorkaround()
{
    return NeedsFenceWorkaround;
}

#endif // HAVE_OPENGL

