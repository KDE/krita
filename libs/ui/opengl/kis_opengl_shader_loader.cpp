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

// Mapping of uniforms to uniform names
std::map<Uniform, const char *> KisShaderProgram::names = {
   {ModelViewProjection, "modelViewProjection"},
   {TextureMatrix, "textureMatrix"},
   {ViewportScale, "viewportScale"},
   {TexelSize, "texelSize"},
   {Texture0, "texture0"},
   {Texture1, "texture1"},
   {FixedLodLevel, "fixedLodLevel"},
   {FragmentColor, "fragColor"}
};

/**
 * Generic shader loading function that will compile a shader program given
 * a vertex shader and fragment shader resource path. Extra code can be prepended
 * to each shader respectively using the header parameters.
 *
 * @param vertPath Resource path to a vertex shader
 * @param fragPath Resource path to a fragment shader
 * @param vertHeader Extra code which will be prepended to the vertex shader
 * @param fragHeader Extra code which will be prepended to the fragment shader
 */
KisShaderProgram *KisOpenGLShaderLoader::loadShader(QString vertPath, QString fragPath,
                                                    QByteArray vertHeader, QByteArray fragHeader)
{
    bool result;

    KisShaderProgram *shader = new KisShaderProgram();

    // Load vertex shader
    QByteArray vertSource;

// XXX Check can be removed and set to the MAC version after we move to Qt5.7
#ifdef Q_OS_OSX
    vertSource.append(KisOpenGL::hasOpenGL3() ? "#version 150 core\n" : "#version 120\n");
    // OpenColorIO doesn't support the new GLSL version yet.
    vertSource.append("#define texture2D texture\n");
    vertSource.append("#define texture3D texture\n");
#else
    if (KisOpenGL::hasOpenGLES()) {
        vertSource.append("#version 300 es\n");
    } else {
        vertSource.append(KisOpenGL::supportsLoD() ? "#version 130\n" : "#version 120\n");
    }
#endif
    vertSource.append(vertHeader);
    QFile vertexShaderFile(":/" + vertPath);
    vertexShaderFile.open(QIODevice::ReadOnly);
    vertSource.append(vertexShaderFile.readAll());

    result = shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource);
    if (!result)
        throw ShaderLoaderException(QString("%1: %2 - Cause: %3").arg("Failed to add vertex shader source from file", vertPath, shader->log()));

    // Load fragment shader
    QByteArray fragSource;

// XXX Check can be removed and set to the MAC version after we move to Qt5.7
#ifdef Q_OS_OSX
    fragSource.append(KisOpenGL::hasOpenGL3() ? "#version 150 core\n" : "#version 120\n");
    // OpenColorIO doesn't support the new GLSL version yet.
    fragSource.append("#define texture2D texture\n");
    fragSource.append("#define texture3D texture\n");
#else
    if (KisOpenGL::hasOpenGLES()) {
        fragSource.append(
                    "#version 300 es\n"
                    "precision mediump float;\n"
                    "precision mediump sampler3D;\n");

        // OpenColorIO doesn't support the new GLSL version yet.
        fragSource.append("#define texture2D texture\n");
        fragSource.append("#define texture3D texture\n");
    } else {
        fragSource.append(KisOpenGL::supportsLoD() ? "#version 130\n" : "#version 120\n");
    }
#endif
    fragSource.append(fragHeader);
    QFile fragmentShaderFile(":/" + fragPath);
    fragmentShaderFile.open(QIODevice::ReadOnly);
    fragSource.append(fragmentShaderFile.readAll());

    result = shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource);
    if (!result)
        throw ShaderLoaderException(QString("%1: %2 - Cause: %3").arg("Failed to add fragment shader source from file", fragPath, shader->log()));

    // Bind attributes
    shader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
    shader->bindAttributeLocation("a_textureCoordinate", PROGRAM_TEXCOORD_ATTRIBUTE);

    // Link
    result = shader->link();
    if (!result)
        throw ShaderLoaderException(QString("Failed to link shader: ").append(vertPath));

    Q_ASSERT(shader->isLinked());

    return shader;
}

/**
 * Specific display shader loading function. It adds the appropriate extra code
 * to the fragment shader depending on what is available on the target machine.
 * Additionally, it picks the appropriate shader files depending on the availability
 * of OpenGL3.
 */
KisShaderProgram *KisOpenGLShaderLoader::loadDisplayShader(QSharedPointer<KisDisplayFilter> displayFilter, bool useHiQualityFiltering)
{
    QByteArray fragHeader;

    if (KisOpenGL::supportsLoD()) {
        fragHeader.append("#define DIRECT_LOD_FETCH\n");
        if (useHiQualityFiltering) {
            fragHeader.append("#define HIGHQ_SCALING\n");
        }
    }

    // If we have an OCIO display filter and it contains a function we add
    // it to our shader header which will sit on top of the fragment code.
    bool haveDisplayFilter = displayFilter && !displayFilter->program().isEmpty();
    if (haveDisplayFilter) {
        fragHeader.append("#define USE_OCIO\n");
        fragHeader.append(displayFilter->program().toLatin1());
    }

    QString vertPath, fragPath;
    // Select appropriate shader files
    if (KisOpenGL::supportsLoD()) {
        vertPath = "matrix_transform.vert";
        fragPath = "highq_downscale.frag";
    } else {
        vertPath = "matrix_transform_legacy.vert";
        fragPath = "simple_texture_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), fragHeader);

    return shader;
}

/**
 * Specific checker shader loading function. It picks the appropriate shader
 * files depending on the availability of OpenGL3 on the target machine.
 */
KisShaderProgram *KisOpenGLShaderLoader::loadCheckerShader()
{
    QString vertPath, fragPath;
    // Select appropriate shader files
    if (KisOpenGL::supportsLoD()) {
        vertPath = "matrix_transform.vert";
        fragPath = "simple_texture.frag";
    } else {
        vertPath = "matrix_transform_legacy.vert";
        fragPath = "simple_texture_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), QByteArray());

    return shader;
}

/**
 * Specific uniform shader loading function. It picks the appropriate shader
 * files depending on the availability of OpenGL3 on the target machine.
 */
KisShaderProgram *KisOpenGLShaderLoader::loadSolidColorShader()
{
    QString vertPath, fragPath;
    // Select appropriate shader files
    if (KisOpenGL::supportsLoD()) {
        vertPath = "matrix_transform.vert";
        fragPath = "solid_color.frag";
    } else {
        vertPath = "matrix_transform_legacy.vert";
        fragPath = "solid_color_legacy.frag";
    }

    KisShaderProgram *shader = loadShader(vertPath, fragPath, QByteArray(), QByteArray());

    return shader;
}
