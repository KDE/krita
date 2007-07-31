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

#include "kis_opengl_shader.h"

#include <QFile>
#include <QVector>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "kis_factory2.h"
#include "kis_opengl.h"
#include "kis_debug_areas.h"

KisOpenGLShader::KisOpenGLShader(GLenum shaderType)
{
    m_valid = false;

    KIS_OPENGL_CLEAR_ERROR();

    m_shader = glCreateShaderObjectARB(shaderType);
    KIS_OPENGL_PRINT_ERROR();

    if (m_shader == 0) {
        kDebug(DBG_AREA_UI) <<"Failed to create shader";
    }
}

KisOpenGLShader::~KisOpenGLShader()
{
    if (m_shader != 0) {
        KIS_OPENGL_CLEAR_ERROR();
        glDeleteObjectARB(m_shader);
        KIS_OPENGL_PRINT_ERROR();
    }
}

void KisOpenGLShader::loadSourceCode(GLsizei numSourceCodeStrings, const GLcharARB **sourceCodeStrings, const GLint *stringLengths)
{
    if (m_shader != 0) {
        KIS_OPENGL_CLEAR_ERROR();
        glShaderSourceARB(m_shader, numSourceCodeStrings, sourceCodeStrings, stringLengths);
        KIS_OPENGL_PRINT_ERROR();

        glCompileShaderARB(m_shader);
        KIS_OPENGL_PRINT_ERROR();

        GLint compiled;

        glGetObjectParameterivARB(m_shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
        KIS_OPENGL_PRINT_ERROR();

        if (compiled) {
            m_valid = true;
        } else {
            kDebug(DBG_AREA_UI) <<"Failed to compile shader";
            kDebug(DBG_AREA_UI) <<"Info log:" << getInfoLog();
        }
    }
}

void KisOpenGLShader::loadSourceCode(QString sourceCodeFilename)
{
    QString fullFilename = KisFactory2::componentData().dirs()->findResource("kis_shaders", sourceCodeFilename);

    if (fullFilename.isNull()) {
        kDebug(DBG_AREA_UI) <<"Failed to find shader source code file:" << sourceCodeFilename;
        return;
    }

    QFile sourceCodeFile(fullFilename);

    if (!sourceCodeFile.open(IO_ReadOnly)) {
        kDebug(DBG_AREA_UI) <<"Unable to open shader source code file:" << fullFilename;
        return;
    }

    QTextStream sourceCodeStream(&sourceCodeFile);
    QVector<QByteArray> sourceCodeStringList;

    while (!sourceCodeStream.atEnd()) {
        sourceCodeStringList.append(sourceCodeStream.readLine().toLatin1());
    }

    QVector<const GLcharARB *> sourceCodeStrings;

    foreach(const QByteArray &sourceString, sourceCodeStringList) {
        sourceCodeStrings.append(sourceString.constData());
    }

    if (sourceCodeStrings.isEmpty()) {
        kDebug(DBG_AREA_UI) <<"Shader source code file is empty:" << fullFilename;
        return;
    }

    loadSourceCode(sourceCodeStrings.count(), &(sourceCodeStrings[0]), NULL);
}

bool KisOpenGLShader::isValid() const
{
    return m_valid;
}

GLhandleARB KisOpenGLShader::handle() const
{
    return m_shader;
}

QString KisOpenGLShader::getInfoLog()
{
    int infoLogLength;
    QString infoLog;

    KIS_OPENGL_CLEAR_ERROR();
    glGetObjectParameterivARB(m_shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);
    KIS_OPENGL_PRINT_ERROR();

    if (infoLogLength > 0) {
        GLcharARB *infoLogBuffer = new GLcharARB[infoLogLength];
        Q_CHECK_PTR(infoLogBuffer);

        glGetInfoLogARB(m_shader, infoLogLength, NULL, infoLogBuffer);
        KIS_OPENGL_PRINT_ERROR();

        infoLog = infoLogBuffer;
        delete [] infoLogBuffer;
    }
    return infoLog;
}

