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

#include "kis_opengl.h"
#include "config-glew.h"

#ifdef HAVE_OPENGL

#ifdef HAVE_GLEW
#include <GL/glew.h>
#endif

#include <QtGlobal>
#ifdef Q_WS_MAC
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <QGLWidget>

#include <kdebug.h>

#include "kis_debug_areas.h"

QGLWidget *KisOpenGL::SharedContextWidget = 0;

void KisOpenGL::createContext()
{
    Q_ASSERT(SharedContextWidget == 0);

    kDebug(DBG_AREA_UI) <<"Creating shared context widget";

    SharedContextWidget = new QGLWidget();//KisOpenGLCanvasFormat);
    SharedContextWidget->makeCurrent();
    initGlew();
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

void KisOpenGL::initGlew()
{
#ifdef HAVE_GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        kDebug(DBG_AREA_UI) <<"glewInit error:" << (const char *)glewGetErrorString(err);
    } else {
        kDebug(DBG_AREA_UI) <<"Status: Using GLEW" << (const char *)glewGetString(GLEW_VERSION);
    }
#endif
}

bool KisOpenGL::hasShadingLanguage()
{
    bool haveShadingLanguage = false;

#ifdef HAVE_GLEW
    if (QGLFormat::hasOpenGL()) {
        makeContextCurrent();

        if (GLEW_ARB_shader_objects && GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader
            && GLEW_ARB_shading_language_100) {
            kDebug(DBG_AREA_UI) <<"Check: have opengl shading extensions";
            haveShadingLanguage = true;
        } else {
            kDebug(DBG_AREA_UI) <<"Check: don't have opengl shading extensions";
        }
    }
#endif
    return haveShadingLanguage;
}

void KisOpenGL::printError(const char *file, int line)
{
    GLenum glErr = glGetError();

    while (glErr != GL_NO_ERROR) {

        kDebug(DBG_AREA_UI) <<"glError:" << (const char *)gluErrorString(glErr);

        if (file != 0) {
            if (line != -1) {
                kDebug(DBG_AREA_UI) <<" at" << file <<" line" << line;
            } else {
                kDebug(DBG_AREA_UI) <<" in" << file;
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

