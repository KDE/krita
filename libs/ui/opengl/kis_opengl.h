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

#include <QtGlobal>
#include <QFlags>

#include "kritaui_export.h"

class QOpenGLContext;
class QString;
class QStringList;
class QSettings;

/**
 * This class manages a shared OpenGL context and provides utility
 * functions for checking capabilities and error reporting.
 */
class KRITAUI_EXPORT KisOpenGL
{
public:
    enum FilterMode {
        NearestFilterMode,  // nearest
        BilinearFilterMode, // linear, no mipmap
        TrilinearFilterMode, // LINEAR_MIPMAP_LINEAR
        HighQualityFiltering // Mipmaps + custom shader
    };
public:

#ifdef Q_OS_WIN
    enum OpenGLRenderer {
        RendererNone = 0x00,
        RendererAuto = 0x01,
        RendererDesktopGL = 0x02,
        RendererAngle = 0x04,
    };
    Q_DECLARE_FLAGS(OpenGLRenderers, OpenGLRenderer);

    // Probe the Windows platform abstraction layer for OpenGL detection
    static void probeWindowsQpaOpenGL(int argc, char **argv, QString userRendererConfigString);

    static OpenGLRenderer getCurrentOpenGLRenderer();
    static OpenGLRenderer getQtPreferredOpenGLRenderer();
    static OpenGLRenderers getSupportedOpenGLRenderers();
    static OpenGLRenderer getUserOpenGLRendererConfig();
    static OpenGLRenderer getNextUserOpenGLRendererConfig();
    static void setNextUserOpenGLRendererConfig(OpenGLRenderer renderer);
    static QString convertOpenGLRendererToConfig(OpenGLRenderer renderer);
    static OpenGLRenderer convertConfigToOpenGLRenderer(QString renderer);
#endif

    /// Request OpenGL version 3.2
    static void initialize();

    /// Initialize shared OpenGL context
    static void initializeContext(QOpenGLContext *ctx);

    static const QString &getDebugText();

    static QStringList getOpenGLWarnings();

    static bool supportsLoD();
    static bool hasOpenGL3();
    static bool hasOpenGLES();

    /// Check for OpenGL
    static bool hasOpenGL();

    /**
     * @brief supportsFilter
     * @return True if OpenGL provides fence sync methods.
     */
    static bool supportsFenceSync();

    /**
     * Returns true if we have a driver that has bugged support to sync objects (a fence)
     * and false otherwise.
     */
    static bool needsFenceWorkaround();

    /**
     * @see a comment in initializeContext()
     */
    static bool needsPixmapCacheWorkaround();

    static void setDefaultFormat(bool enableDebug = false, bool debugSynchronous = false, QSettings *kritadisplayrc = 0);

private:


    KisOpenGL();


};

#ifdef Q_OS_WIN
Q_DECLARE_OPERATORS_FOR_FLAGS(KisOpenGL::OpenGLRenderers);
#endif

#endif // KIS_OPENGL_H_
