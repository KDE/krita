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
#ifndef KIS_OPENGL_FRAGMENT_SHADER_H_
#define KIS_OPENGL_FRAGMENT_SHADER_H_

#include <QString>
#include <QVector>

#include "krita_export.h"
#include "kis_shared_ptr.h"
#include "opengl/kis_opengl_shader.h"

class KisOpenGLFragmentShader;
typedef KisSharedPtr<KisOpenGLFragmentShader> KisOpenGLFragmentShaderSP;

/**
 * An encapsulation of an OpenGL Shading Language fragment shader object.
 */
class KRITAUI_EXPORT KisOpenGLFragmentShader : public KisOpenGLShader
{
public:

    /**
     * Loads the shader source code from the given file, which will be searched
     * for in the 'kis_shaders' resource directory (krita/data/shaders).
     * Refer to KisOpenGLShader for more information.
     * @param sourceCodeFilename the file to read the source code from
     */
    static KisOpenGLFragmentShader *createFromSourceCodeFile(const QString& sourceCodeFilename);

    /**
     * Loads the shader source code from the given QString
     *
     * @param sourceCodeStrings the source string to load from
     */
    static KisOpenGLFragmentShader *createFromSourceCodeString(const QString& sourceCodeString);

protected:
    KisOpenGLFragmentShader();
};

#endif // KIS_OPENGL_FRAGMENT_SHADER_H_

