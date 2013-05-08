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

#include <QGLWidget>

#include <kis_debug.h>

namespace
{
    QGLWidget *SharedContextWidget = 0;
}

void KisOpenGL::createContext()
{
    Q_ASSERT(SharedContextWidget == 0);

    dbgUI << "OpenGL: Creating shared context widget";

    SharedContextWidget = new QGLWidget();
    SharedContextWidget->makeCurrent();
}

void KisOpenGL::makeContextCurrent()
{
    sharedContextWidget()->makeCurrent();
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

#endif // HAVE_OPENGL

