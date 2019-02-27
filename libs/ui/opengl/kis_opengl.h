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

#include <QSurfaceFormat>
#include "kis_config.h"

#include "kritaui_export.h"

class QOpenGLContext;
class QString;
class QStringList;
class QSurfaceFormat;

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

    enum OpenGLRenderer {
        RendererNone = 0x00,
        RendererAuto = 0x01,
        RendererDesktopGL = 0x02,
        RendererOpenGLES = 0x04,
    };
    Q_DECLARE_FLAGS(OpenGLRenderers, OpenGLRenderer);

    static QSurfaceFormat selectSurfaceFormat(KisOpenGL::OpenGLRenderer preferredRenderer,
                                              KisConfig::RootSurfaceFormat preferredRootSurfaceFormat,
                                              bool enableDebug);

    static void setDefaultSurfaceFormat(const QSurfaceFormat &format);

    static OpenGLRenderer getCurrentOpenGLRenderer();
    static OpenGLRenderer getQtPreferredOpenGLRenderer();
    static OpenGLRenderers getSupportedOpenGLRenderers();
    static OpenGLRenderer getUserPreferredOpenGLRendererConfig();
    static void setUserPreferredOpenGLRendererConfig(OpenGLRenderer renderer);
    static QString convertOpenGLRendererToConfig(OpenGLRenderer renderer);
    static OpenGLRenderer convertConfigToOpenGLRenderer(QString renderer);

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

    static void testingInitializeDefaultSurfaceFormat();
    static void setDebugSynchronous(bool value);

private:
    static void fakeInitWindowsOpenGL(KisOpenGL::OpenGLRenderers supportedRenderers, KisOpenGL::OpenGLRenderer preferredByQt);

    KisOpenGL();


};

#ifdef Q_OS_WIN
Q_DECLARE_OPERATORS_FOR_FLAGS(KisOpenGL::OpenGLRenderers);
#endif

#endif // KIS_OPENGL_H_
