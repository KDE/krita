/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2008 Tom Burdick <thomas.burdick@gmail.com>
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

#include "opengl/kis_opengl_shader.h"


#include <QFile>
#include <QVector>

#include <kcomponentdata.h>
#include <kstandarddirs.h>

#include "kis_factory2.h"
#include "kis_debug.h"

KisOpenGLShader::KisOpenGLShader(GLenum shaderType)
{
    m_valid = false;

    KIS_OPENGL_CLEAR_ERROR();

    m_shader = glCreateShader(shaderType);
    KIS_OPENGL_PRINT_ERROR();

    if (m_shader == 0) {
        dbgUI << "Failed to create shader";
    }
}

KisOpenGLShader::~KisOpenGLShader()
{
    if (m_shader != 0) {
        KIS_OPENGL_CLEAR_ERROR();
        glDeleteShader(m_shader);
        KIS_OPENGL_PRINT_ERROR();
    }
}

void KisOpenGLShader::loadSourceCodeFromCStrings(GLsizei numSourceCodeStrings, const GLchar **sourceCodeStrings, const GLint *stringLengths)
{
    if (m_shader != 0) {
        KIS_OPENGL_CLEAR_ERROR();
        glShaderSource(m_shader, numSourceCodeStrings, sourceCodeStrings, stringLengths);
        KIS_OPENGL_PRINT_ERROR();

        glCompileShader(m_shader);
        KIS_OPENGL_PRINT_ERROR();

        GLint compiled;

        glGetShaderiv(m_shader, GL_COMPILE_STATUS, &compiled);
        KIS_OPENGL_PRINT_ERROR();

        if (compiled) {
            m_valid = true;
        } else {
            dbgUI << "Failed to compile shader";
            dbgUI << "Info log:" << getInfoLog();
        }
    }
}

void KisOpenGLShader::loadSourceCodeFromFile(const QString & sourceCodeFilename)
{
    QString fullFilename = KisFactory2::componentData().dirs()->findResource("kis_shaders", sourceCodeFilename);

    if (fullFilename.isNull()) {
        dbgUI << "Failed to find shader source code file:" << sourceCodeFilename;
        return;
    }

    QFile sourceCodeFile(fullFilename);

    if (!sourceCodeFile.open(QIODevice::ReadOnly)) {
        dbgUI << "Unable to open shader source code file:" << fullFilename;
        return;
    }

    QTextStream sourceCodeStream(&sourceCodeFile);
    QVector<QByteArray> sourceCodeStringList;

    while (!sourceCodeStream.atEnd()) {
        sourceCodeStringList.append(sourceCodeStream.readLine().toLatin1());
    }

    QVector<const GLchar *> sourceCodeStrings;

    foreach(const QByteArray &sourceString, sourceCodeStringList) {
        sourceCodeStrings.append(sourceString.constData());
    }

    if (sourceCodeStrings.isEmpty()) {
        dbgUI << "Shader source code file is empty:" << fullFilename;
        return;
    }

    loadSourceCodeFromCStrings(sourceCodeStrings.count(), &(sourceCodeStrings[0]), NULL);
}

void KisOpenGLShader::loadSourceCodeFromQString(QString sourceCodeString)
{
    QByteArray string = sourceCodeString.toAscii();

    if (string.length() == 0) {
        dbgUI << "Shader source code vector is empty";
        return;
    }
    GLint length = string.length();
    GLsizei size = 1;
    const char* cstring = string.constData();
    loadSourceCodeFromCStrings(size, &(cstring), &length);
}

bool KisOpenGLShader::isValid() const
{
    return m_valid;
}

GLuint KisOpenGLShader::handle() const
{
    return m_shader;
}

QString KisOpenGLShader::getInfoLog()
{
    GLint infoLogLength;
    QString infoLog;

    KIS_OPENGL_CLEAR_ERROR();
    glGetShaderiv(m_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    KIS_OPENGL_PRINT_ERROR();

    if (infoLogLength > 0) {
        GLchar *infoLogBuffer = new GLchar[infoLogLength];
        Q_CHECK_PTR(infoLogBuffer);

        glGetShaderInfoLog(m_shader, infoLogLength, NULL, infoLogBuffer);
        KIS_OPENGL_PRINT_ERROR();

        infoLog = infoLogBuffer;
        delete [] infoLogBuffer;
    }
    return infoLog;
}

