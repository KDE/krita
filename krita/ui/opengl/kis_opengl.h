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

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include <QtGlobal>
class QOpenGLContext;

#include "kritaui_export.h"


/**
 * This class manages a shared OpenGL context and provides utility
 * functions for checking capabilities and error reporting.
 */
class KRITAUI_EXPORT KisOpenGL
{
public:

    /// Request OpenGL version 3.2
    static void initialize();

    /// Initialize shared OpenGL context
    static int initializeContext(QOpenGLContext* s);

    /// Check for OpenGL
    static bool hasOpenGL();

    /**
     * @brief supportsGLSL13
     * @return true if we have a modern opengl capable of high-quality filtering
     */
    static bool supportsGLSL13();

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

private:
    KisOpenGL();


};

#endif // KIS_OPENGL_H_

