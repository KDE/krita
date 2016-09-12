/* This file is part of the KDE project
 * Copyright (C) Julian Thijssen <julianthijssen@gmail.com>, (C) 2016
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

#include "kis_opengl_shader_loader.h"

#include "opengl/kis_opengl.h"
#include "kis_config.h"

#include <QFile>
#include <QMessageBox>

#include <KLocalizedString>

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

KisOpenGLShaderLoader::KisOpenGLShaderLoader()
{

}

KisShaderProgram *KisOpenGLShaderLoader::loadShader(QString vertPath, QString fragPath,
                                                       QByteArray vertHeader, QByteArray fragHeader)
{
    bool result;

    KisShaderProgram *shader = new KisShaderProgram();

    // Load vertex shader
    QByteArray vertSource;

    vertSource.append(KisOpenGL::hasOpenGL3() ? "#version 150 core\n" : "#version 120");
    vertSource.append(vertHeader);
    QFile vertexShaderFile(":/" + vertPath);
    vertexShaderFile.open(QIODevice::ReadOnly);
    vertSource.append(vertexShaderFile.readAll());

    result = shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource);
    if (!result)
        throw ShaderLoaderException(QString("Failed to add vertex shader source for file: ").append(vertPath));

    // Load fragment shader
    QByteArray fragSource;

    fragSource.append(KisOpenGL::hasOpenGL3() ? "#version 150 core\n" : "#version 120");
    fragSource.append(fragHeader);
    QFile fragmentShaderFile(":/" + fragPath);
    fragmentShaderFile.open(QIODevice::ReadOnly);
    fragSource.append(fragmentShaderFile.readAll());

    result = shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource);
    if (!result)
        throw ShaderLoaderException(QString("Failed to add fragment shader source for file: ").append(fragPath));

    // Bind uniforms
    shader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
    shader->bindAttributeLocation("a_textureCoordinate", PROGRAM_TEXCOORD_ATTRIBUTE);

    // Link
    result = shader->link();
    if (!result)
        throw ShaderLoaderException(QString("Failed to link shader: ").append(vertPath));

    Q_ASSERT(shader->isLinked());

    return shader;
}

KisShaderProgram *KisOpenGLShaderLoader::loadDisplayShader(KisDisplayFilter *displayFilter, bool useHiQualityFiltering)
{
    QByteArray fragHeader;

    if (KisOpenGL::hasOpenGL3()) {
        fragHeader.append("#define DIRECT_LOD_FETCH\n");
        if (useHiQualityFiltering) {
            fragHeader.append("#define HIGHQ_SCALING\n");
        }
    }

    bool haveDisplayFilter = displayFilter && !displayFilter->program().isEmpty();
    if (haveDisplayFilter) {
        fragHeader.append("#define USE_OCIO\n");
        fragHeader.append(displayFilter->program().toLatin1());
    }

    QString vertPath, fragPath;
    // Select appropriate vertex shader
    if (KisOpenGL::hasOpenGL3()) {
        vertPath = "matrix_transform.vert";
        fragPath = "highq_downscale.frag";
    } else {
        vertPath = "matrix_transform_legacy.vert";
        fragPath = "simple_texture_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), fragHeader);

    return shader;
}

KisShaderProgram *KisOpenGLShaderLoader::loadCheckerShader()
{
    QString vertPath, fragPath;
    // Select appropriate vertex shader
    if (KisOpenGL::hasOpenGL3()) {
        vertPath = "matrix_transform.vert";
        fragPath = "simple_texture.frag";
    } else {
        vertPath = "matrix_transform_legacy.vert";
        fragPath = "simple_texture_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), QByteArray());

    return shader;
}

KisShaderProgram *KisOpenGLShaderLoader::loadCursorShader()
{
    QString vertPath, fragPath;
    // Select appropriate vertex shader
    if (KisOpenGL::hasOpenGL3()) {
        vertPath = "cursor.vert";
        fragPath = "cursor.frag";
    } else {
        vertPath = "cursor_legacy.vert";
        fragPath = "cursor_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), QByteArray());

    return shader;
}
