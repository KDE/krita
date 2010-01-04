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

#include "opengl/kis_opengl_program.h"

#include <QPointF>

#include "opengl/kis_opengl_shader.h"
#include "kis_debug.h"

KisOpenGLProgram::KisOpenGLProgram()
        : m_isValid(false)
{
    KIS_OPENGL_CLEAR_ERROR();

    m_program = glCreateProgramObjectARB();
    KIS_OPENGL_PRINT_ERROR();

    if (m_program == 0) {
        dbgUI << "Failed to create program";
    }
}

KisOpenGLProgram::~KisOpenGLProgram()
{
    if (m_program != 0) {
        KIS_OPENGL_CLEAR_ERROR();
        glDeleteObjectARB(m_program);
        KIS_OPENGL_PRINT_ERROR();
    }
}

GLhandleARB KisOpenGLProgram::handle() const
{
    return m_program;
}

GLint KisOpenGLProgram::uniformVariableLocation(const GLchar *variableName) const
{
    GLint location = -1;

    if (m_program != 0) {
        KIS_OPENGL_CLEAR_ERROR();
        location = glGetUniformLocationARB(m_program, variableName);
        KIS_OPENGL_PRINT_ERROR();

        if (location == -1) {
            dbgUI << "Failed to find uniform variable '" << variableName << "' in program";
        }
    }

    return location;
}

void KisOpenGLProgram::setUniformVariable(const GLchar *variableName, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) const
{
    if (m_program != 0) {
        GLint location = uniformVariableLocation(variableName);

        if (location != -1) {
            KIS_OPENGL_CLEAR_ERROR();
            glUniform4fARB(location, v0, v1, v2, v3);
            KIS_OPENGL_PRINT_ERROR();
        }
    }
}

void KisOpenGLProgram::setUniformVariable(const GLchar *variableName, GLfloat v0, GLfloat v1) const
{
    if (m_program != 0) {
        GLint location = uniformVariableLocation(variableName);

        if (location != -1) {
            KIS_OPENGL_CLEAR_ERROR();
            glUniform2fARB(location, v0, v1);
            KIS_OPENGL_PRINT_ERROR();
        }
    }
}

void KisOpenGLProgram::setUniformVariable(const GLchar *variableName, const QPointF &value) const
{
    setUniformVariable(variableName, value.x(), value.y());
}

void KisOpenGLProgram::setUniformVariable(const GLchar *variableName, GLfloat value) const
{
    if (m_program != 0) {
        GLint location = uniformVariableLocation(variableName);

        if (location != -1) {
            KIS_OPENGL_CLEAR_ERROR();
            glUniform1fARB(location, value);
            KIS_OPENGL_PRINT_ERROR();
        }
    }
}

void KisOpenGLProgram::setUniformVariable(const GLchar *variableName, GLint i) const
{
    if (m_program != 0) {
        GLint location = uniformVariableLocation(variableName);

        if (location != -1) {
            KIS_OPENGL_CLEAR_ERROR();
            glUniform1iARB(location, i);
            KIS_OPENGL_PRINT_ERROR();
        }
    }
}

void KisOpenGLProgram::attachShader(KisOpenGLShader& shader)
{
    if (m_program != 0 && shader.isValid()) {
        KIS_OPENGL_CLEAR_ERROR();
        glAttachObjectARB(m_program, shader.handle());
        KIS_OPENGL_PRINT_ERROR();
    }
}

void KisOpenGLProgram::detachShader(KisOpenGLShader& shader)
{
    if (m_program != 0 && shader.isValid()) {
        KIS_OPENGL_CLEAR_ERROR();
        glDetachObjectARB(m_program, shader.handle());
        KIS_OPENGL_PRINT_ERROR();
    }
}

void KisOpenGLProgram::link()
{
    if (m_program != 0) {
        KIS_OPENGL_CLEAR_ERROR();
        glLinkProgramARB(m_program);
        KIS_OPENGL_PRINT_ERROR();

        GLint linked;

        glGetObjectParameterivARB(m_program, GL_OBJECT_LINK_STATUS_ARB, &linked);
        KIS_OPENGL_PRINT_ERROR();

        if (linked) {
            m_isValid = true;
        } else {
            m_isValid = false;
            dbgUI << "Failed to link program";
            dbgUI << "Info log:" << getInfoLog();
        }
    }
}

QString KisOpenGLProgram::getInfoLog() const
{
    QString infoLog;

    if (m_program != 0) {
        GLint infoLogLength;

        KIS_OPENGL_CLEAR_ERROR();
        glGetObjectParameterivARB(m_program, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
        KIS_OPENGL_PRINT_ERROR();

        if (infoLogLength > 0) {
            GLcharARB *infoLogBuffer = new GLcharARB[infoLogLength];
            Q_CHECK_PTR(infoLogBuffer);

            glGetInfoLogARB(m_program, infoLogLength, NULL, infoLogBuffer);
            KIS_OPENGL_PRINT_ERROR();

            infoLog = infoLogBuffer;
            delete [] infoLogBuffer;
        }
    }
    return infoLog;
}

void KisOpenGLProgram::activate()
{
    if (m_program != 0 && m_isValid) {
        KIS_OPENGL_CLEAR_ERROR();
        glUseProgramObjectARB(m_program);
        KIS_OPENGL_PRINT_ERROR();
    }
}

void KisOpenGLProgram::deactivate()
{
    KIS_OPENGL_CLEAR_ERROR();
    glUseProgramObjectARB(0);
    KIS_OPENGL_PRINT_ERROR();
}

bool KisOpenGLProgram::active() const
{
    if (m_program != 0 && m_isValid) {

        KIS_OPENGL_CLEAR_ERROR();
        GLhandleARB activeProgram = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
        KIS_OPENGL_PRINT_ERROR();

        if (activeProgram == m_program) {
            return true;
        }
    }
    return false;
}

bool KisOpenGLProgram::isValid() const
{
    return m_isValid;
}

