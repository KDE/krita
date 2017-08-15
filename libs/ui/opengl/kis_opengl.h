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

#include <KoConfig.h>

#include <QtGlobal>
class QOpenGLContext;
class QString;

#include "kritaui_export.h"


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

    /// Request OpenGL version 3.2
    static void initialize();

    /// Initialize shared OpenGL context
    static void initializeContext(QOpenGLContext *ctx);

    static const QString &getDebugText();

    static bool supportsLoD();
    static bool hasOpenGL3();

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

    static void setDefaultFormat(bool enableDebug = false, bool debugSynchronous = false);

private:


    KisOpenGL();


};

#endif // KIS_OPENGL_H_
