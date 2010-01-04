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

#include "opengl/kis_opengl_vertex_shader.h"


#include <GL/glew.h>

KisOpenGLVertexShader *KisOpenGLVertexShader::createFromSourceCodeFile(const QString& sourceCodeFilename)
{
    KisOpenGLVertexShader *shader = new KisOpenGLVertexShader();
    shader->loadSourceCodeFromFile(sourceCodeFilename);
    return shader;
}

KisOpenGLVertexShader *KisOpenGLVertexShader::createFromSourceCodeString(const QString& sourceCodeString)
{
    KisOpenGLVertexShader *shader = new KisOpenGLVertexShader();
    shader->loadSourceCodeFromQString(sourceCodeString);
    return shader;
}

KisOpenGLVertexShader::KisOpenGLVertexShader()
        : KisOpenGLShader(GL_VERTEX_SHADER)
{
}

