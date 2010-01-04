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

#include "opengl/kis_opengl_fragment_shader.h"


#include <GL/glew.h>

KisOpenGLFragmentShader *KisOpenGLFragmentShader::createFromSourceCodeFile(const QString& sourceCodeFilename)
{
    KisOpenGLFragmentShader *shader = new KisOpenGLFragmentShader();
    shader->loadSourceCodeFromFile(sourceCodeFilename);
    return shader;
}

KisOpenGLFragmentShader *KisOpenGLFragmentShader::createFromSourceCodeString(const QString& sourceCodeString)
{
    KisOpenGLFragmentShader *shader = new KisOpenGLFragmentShader();
    shader->loadSourceCodeFromQString(sourceCodeString);
    return shader;
}

KisOpenGLFragmentShader::KisOpenGLFragmentShader() :
        KisOpenGLShader(GL_FRAGMENT_SHADER)
{
}
