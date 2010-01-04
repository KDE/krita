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
#ifndef KIS_OPENGL_H_
#define KIS_OPENGL_H_

/** @file */

#include <config-opengl.h>
#include <config-glew.h>

#ifdef HAVE_OPENGL

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#ifdef HAVE_GLEW
#include <GL/glew.h>
#endif

#include <QtGlobal>
#ifdef Q_WS_MAC
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "krita_export.h"

class QGLWidget;

/**
 * This class manages a shared OpenGL context and provides utility
 * functions for checking capabilities and error reporting.
 */
class KRITAUI_EXPORT KisOpenGL
{
public:
    /**
     * Returns the QGLWidget that uses the shared OpenGL context.
     * You should pass this as the shareWidget parameter to the
     * QGLWidget constructor.
     */
    static QGLWidget *sharedContextWidget();

    /**
     * Make the shared OpenGL context the current context. You should
     * make the context current before creating textures, display lists,
     * shader objects, etc, that are to be shared by multiple QGLWidgets.
     */
    static void makeContextCurrent();

    /**
     * Returns true if the OpenGL shading language is available
     * (using the core API, i.e. OpenGL version is 2.0 or greater).
     */
    static bool hasShadingLanguage();

    /**
     * Print any error messages waiting to be read from glGetError(). Use
     * the helper macro KIS_OPENGL_PRINT_ERROR() to generate the source
     * file and line number to identify the location the error is reported
     * from.
     */
    static void printError(const char *file = 0, int line = -1);

    /**
     * Clear any error codes waiting to be read from glGetError().
     */
    static void clearError();

private:
    KisOpenGL();

    static void createContext();
    static void initGlew();
};

/**
 * Helper macro to print out any OpenGL error messages waiting to be
 * read. This will also print the source file and line number where
 * the print is performed.
 */
#define KIS_OPENGL_PRINT_ERROR() KisOpenGL::printError(__FILE__, __LINE__)

/**
 * Helper macro to clear out any OpenGL error messages waiting to be
 * read.
 */
#define KIS_OPENGL_CLEAR_ERROR() KisOpenGL::clearError()

#endif // HAVE_OPENGL

#endif // KIS_OPENGL_H_

