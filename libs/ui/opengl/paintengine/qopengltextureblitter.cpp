/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qopengltextureblitter_p.h"

#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#endif

QT_BEGIN_NAMESPACE

static const char vertex_shader150[] =
    "#version 150 core\n"
    "in vec3 vertexCoord;"
    "in vec2 textureCoord;"
    "out vec2 uv;"
    "uniform mat4 vertexTransform;"
    "uniform mat3 textureTransform;"
    "void main() {"
    "   uv = (textureTransform * vec3(textureCoord,1.0)).xy;"
    "   gl_Position = vertexTransform * vec4(vertexCoord,1.0);"
    "}";

static const char fragment_shader150[] =
    "#version 150 core\n"
    "in vec2 uv;"
    "out vec4 fragcolor;"
    "uniform sampler2D textureSampler;"
    "uniform bool swizzle;"
    "uniform float opacity;"
    "void main() {"
    "   vec4 tmpFragColor = texture(textureSampler, uv);"
    "   tmpFragColor.a *= opacity;"
    "   fragcolor = swizzle ? tmpFragColor.bgra : tmpFragColor;"
    "}";

static const char vertex_shader[] =
    "attribute highp vec3 vertexCoord;"
    "attribute highp vec2 textureCoord;"
    "varying highp vec2 uv;"
    "uniform highp mat4 vertexTransform;"
    "uniform highp mat3 textureTransform;"
    "void main() {"
    "   uv = (textureTransform * vec3(textureCoord,1.0)).xy;"
    "   gl_Position = vertexTransform * vec4(vertexCoord,1.0);"
    "}";

static const char fragment_shader[] =
    "varying highp vec2 uv;"
    "uniform sampler2D textureSampler;"
    "uniform bool swizzle;"
    "uniform highp float opacity;"
    "void main() {"
    "   highp vec4 tmpFragColor = texture2D(textureSampler,uv);"
    "   tmpFragColor.a *= opacity;"
    "   gl_FragColor = swizzle ? tmpFragColor.bgra : tmpFragColor;"
    "}";

static const char fragment_shader_external_oes[] =
    "#extension GL_OES_EGL_image_external : require\n"
    "varying highp vec2 uv;"
    "uniform samplerExternalOES textureSampler;\n"
    "uniform bool swizzle;"
    "uniform highp float opacity;"
    "void main() {"
    "   highp vec4 tmpFragColor = texture2D(textureSampler, uv);"
    "   tmpFragColor.a *= opacity;"
    "   gl_FragColor = swizzle ? tmpFragColor.bgra : tmpFragColor;"
    "}";

static const GLfloat vertex_buffer_data[] = {
        -1,-1, 0,
        -1, 1, 0,
         1,-1, 0,
        -1, 1, 0,
         1,-1, 0,
         1, 1, 0
};

static const GLfloat texture_buffer_data[] = {
        0, 0,
        0, 1,
        1, 0,
        0, 1,
        1, 0,
        1, 1
};

class TextureBinder
{
public:
    TextureBinder(GLenum target, GLuint textureId) : m_target(target)
    {
        QOpenGLContext::currentContext()->functions()->glBindTexture(m_target, textureId);
    }
    ~TextureBinder()
    {
        QOpenGLContext::currentContext()->functions()->glBindTexture(m_target, 0);
    }

private:
    GLenum m_target;
};

class QOpenGLTextureBlitterPrivate
{
public:
    enum TextureMatrixUniform {
        User,
        Identity,
        IdentityFlipped
    };

    enum ProgramIndex {
        TEXTURE_2D,
        TEXTURE_EXTERNAL_OES
    };

    QOpenGLTextureBlitterPrivate() :
        swizzle(false),
        opacity(1.0f),
        vao(new QOpenGLVertexArrayObject),
        currentTarget(TEXTURE_2D)
    { }

    bool buildProgram(ProgramIndex idx, const char *vs, const char *fs);

    void blit(GLuint texture, const QMatrix4x4 &vertexTransform, const QMatrix3x3 &textureTransform);
    void blit(GLuint texture, const QMatrix4x4 &vertexTransform, QOpenGLTextureBlitter::Origin origin);

    void prepareProgram(const QMatrix4x4 &vertexTransform);

    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer textureBuffer;
    struct Program {
        Program() :
            vertexCoordAttribPos(0),
            vertexTransformUniformPos(0),
            textureCoordAttribPos(0),
            textureTransformUniformPos(0),
            swizzleUniformPos(0),
            opacityUniformPos(0),
            swizzle(false),
            opacity(0.0f),
            textureMatrixUniformState(User)
        { }
        QScopedPointer<QOpenGLShaderProgram> glProgram;
        GLuint vertexCoordAttribPos;
        GLuint vertexTransformUniformPos;
        GLuint textureCoordAttribPos;
        GLuint textureTransformUniformPos;
        GLuint swizzleUniformPos;
        GLuint opacityUniformPos;
        bool swizzle;
        float opacity;
        TextureMatrixUniform textureMatrixUniformState;
    } programs[2];
    bool swizzle;
    float opacity;
    QScopedPointer<QOpenGLVertexArrayObject> vao;
    GLenum currentTarget;
};

static inline QOpenGLTextureBlitterPrivate::ProgramIndex targetToProgramIndex(GLenum target)
{
    switch (target) {
    case GL_TEXTURE_2D:
        return QOpenGLTextureBlitterPrivate::TEXTURE_2D;
    case GL_TEXTURE_EXTERNAL_OES:
        return QOpenGLTextureBlitterPrivate::TEXTURE_EXTERNAL_OES;
    default:
        qWarning("Unsupported texture target 0x%x", target);
        return QOpenGLTextureBlitterPrivate::TEXTURE_2D;
    }
}

void QOpenGLTextureBlitterPrivate::prepareProgram(const QMatrix4x4 &vertexTransform)
{
    Program *program = &programs[targetToProgramIndex(currentTarget)];

    vertexBuffer.bind();
    program->glProgram->setAttributeBuffer(program->vertexCoordAttribPos, GL_FLOAT, 0, 3, 0);
    program->glProgram->enableAttributeArray(program->vertexCoordAttribPos);
    vertexBuffer.release();

    program->glProgram->setUniformValue(program->vertexTransformUniformPos, vertexTransform);

    textureBuffer.bind();
    program->glProgram->setAttributeBuffer(program->textureCoordAttribPos, GL_FLOAT, 0, 2, 0);
    program->glProgram->enableAttributeArray(program->textureCoordAttribPos);
    textureBuffer.release();

    if (swizzle != program->swizzle) {
        program->glProgram->setUniformValue(program->swizzleUniformPos, swizzle);
        program->swizzle = swizzle;
    }

    if (opacity != program->opacity) {
        program->glProgram->setUniformValue(program->opacityUniformPos, opacity);
        program->opacity = opacity;
    }
}

void QOpenGLTextureBlitterPrivate::blit(GLuint texture,
                                        const QMatrix4x4 &vertexTransform,
                                        const QMatrix3x3 &textureTransform)
{
    TextureBinder binder(currentTarget, texture);
    prepareProgram(vertexTransform);

    Program *program = &programs[targetToProgramIndex(currentTarget)];
    program->glProgram->setUniformValue(program->textureTransformUniformPos, textureTransform);
    program->textureMatrixUniformState = User;

    QOpenGLContext::currentContext()->functions()->glDrawArrays(GL_TRIANGLES, 0, 6);
}

void QOpenGLTextureBlitterPrivate::blit(GLuint texture,
                                        const QMatrix4x4 &vertexTransform,
                                        QOpenGLTextureBlitter::Origin origin)
{
    TextureBinder binder(currentTarget, texture);
    prepareProgram(vertexTransform);

    Program *program = &programs[targetToProgramIndex(currentTarget)];
    if (origin == QOpenGLTextureBlitter::OriginTopLeft) {
        if (program->textureMatrixUniformState != IdentityFlipped) {
            QMatrix3x3 flipped;
            flipped(1,1) = -1;
            flipped(1,2) = 1;
            program->glProgram->setUniformValue(program->textureTransformUniformPos, flipped);
            program->textureMatrixUniformState = IdentityFlipped;
        }
    } else if (program->textureMatrixUniformState != Identity) {
        program->glProgram->setUniformValue(program->textureTransformUniformPos, QMatrix3x3());
        program->textureMatrixUniformState = Identity;
    }

    QOpenGLContext::currentContext()->functions()->glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool QOpenGLTextureBlitterPrivate::buildProgram(ProgramIndex idx, const char *vs, const char *fs)
{
    Program *p = &programs[idx];

    p->glProgram.reset(new QOpenGLShaderProgram);

    p->glProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    p->glProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
    p->glProgram->link();
    if (!p->glProgram->isLinked()) {
        qWarning() << "Could not link shader program:\n" << p->glProgram->log();
        return false;
    }

    p->glProgram->bind();

    p->vertexCoordAttribPos = p->glProgram->attributeLocation("vertexCoord");
    p->vertexTransformUniformPos = p->glProgram->uniformLocation("vertexTransform");
    p->textureCoordAttribPos = p->glProgram->attributeLocation("textureCoord");
    p->textureTransformUniformPos = p->glProgram->uniformLocation("textureTransform");
    p->swizzleUniformPos = p->glProgram->uniformLocation("swizzle");
    p->opacityUniformPos = p->glProgram->uniformLocation("opacity");

    p->glProgram->setUniformValue(p->swizzleUniformPos, false);

    return true;
}

QOpenGLTextureBlitter::QOpenGLTextureBlitter()
    : d_ptr(new QOpenGLTextureBlitterPrivate)
{
}

QOpenGLTextureBlitter::~QOpenGLTextureBlitter()
{
    destroy();
}

bool QOpenGLTextureBlitter::create()
{
    QOpenGLContext *currentContext = QOpenGLContext::currentContext();
    if (!currentContext)
        return false;

    Q_D(QOpenGLTextureBlitter);

    if (d->programs[QOpenGLTextureBlitterPrivate::TEXTURE_2D].glProgram)
        return true;

    QSurfaceFormat format = currentContext->format();
    if (format.profile() == QSurfaceFormat::CoreProfile && format.version() >= qMakePair(3,2)) {
        if (!d->buildProgram(QOpenGLTextureBlitterPrivate::TEXTURE_2D, vertex_shader150, fragment_shader150))
            return false;
    } else {
        if (!d->buildProgram(QOpenGLTextureBlitterPrivate::TEXTURE_2D, vertex_shader, fragment_shader))
            return false;
        if (supportsExternalOESTarget())
            if (!d->buildProgram(QOpenGLTextureBlitterPrivate::TEXTURE_EXTERNAL_OES, vertex_shader, fragment_shader_external_oes))
                return false;
    }

    // Create and bind the VAO, if supported.
    QOpenGLVertexArrayObject::Binder vaoBinder(d->vao.data());

    d->vertexBuffer.create();
    d->vertexBuffer.bind();
    d->vertexBuffer.allocate(vertex_buffer_data, sizeof(vertex_buffer_data));
    d->vertexBuffer.release();

    d->textureBuffer.create();
    d->textureBuffer.bind();
    d->textureBuffer.allocate(texture_buffer_data, sizeof(texture_buffer_data));
    d->textureBuffer.release();

    return true;
}

bool QOpenGLTextureBlitter::isCreated() const
{
    Q_D(const QOpenGLTextureBlitter);
    return d->programs[QOpenGLTextureBlitterPrivate::TEXTURE_2D].glProgram;
}

void QOpenGLTextureBlitter::destroy()
{
    if (!isCreated())
        return;
    Q_D(QOpenGLTextureBlitter);
    d->programs[QOpenGLTextureBlitterPrivate::TEXTURE_2D].glProgram.reset();
    d->programs[QOpenGLTextureBlitterPrivate::TEXTURE_EXTERNAL_OES].glProgram.reset();
    d->vertexBuffer.destroy();
    d->textureBuffer.destroy();
    d->vao.reset();
}

bool QOpenGLTextureBlitter::supportsExternalOESTarget() const
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    return ctx && ctx->isOpenGLES() && ctx->hasExtension("GL_OES_EGL_image_external");
}

void QOpenGLTextureBlitter::bind(GLenum target)
{
    Q_D(QOpenGLTextureBlitter);

    if (d->vao->isCreated())
        d->vao->bind();

    d->currentTarget = target;
    QOpenGLTextureBlitterPrivate::Program *p = &d->programs[targetToProgramIndex(target)];
    p->glProgram->bind();

    d->vertexBuffer.bind();
    p->glProgram->setAttributeBuffer(p->vertexCoordAttribPos, GL_FLOAT, 0, 3, 0);
    p->glProgram->enableAttributeArray(p->vertexCoordAttribPos);
    d->vertexBuffer.release();

    d->textureBuffer.bind();
    p->glProgram->setAttributeBuffer(p->textureCoordAttribPos, GL_FLOAT, 0, 2, 0);
    p->glProgram->enableAttributeArray(p->textureCoordAttribPos);
    d->textureBuffer.release();
}

void QOpenGLTextureBlitter::release()
{
    Q_D(QOpenGLTextureBlitter);
    d->programs[targetToProgramIndex(d->currentTarget)].glProgram->release();
    if (d->vao->isCreated())
        d->vao->release();
}

void QOpenGLTextureBlitter::setSwizzleRB(bool swizzle)
{
    Q_D(QOpenGLTextureBlitter);
    d->swizzle = swizzle;
}

void QOpenGLTextureBlitter::setOpacity(float opacity)
{
    Q_D(QOpenGLTextureBlitter);
    d->opacity = opacity;
}

void QOpenGLTextureBlitter::blit(GLuint texture,
                                 const QMatrix4x4 &targetTransform,
                                 Origin sourceOrigin)
{
    Q_D(QOpenGLTextureBlitter);
    d->blit(texture,targetTransform, sourceOrigin);
}

void QOpenGLTextureBlitter::blit(GLuint texture,
                                 const QMatrix4x4 &targetTransform,
                                 const QMatrix3x3 &sourceTransform)
{
    Q_D(QOpenGLTextureBlitter);
    d->blit(texture, targetTransform, sourceTransform);
}

QMatrix4x4 QOpenGLTextureBlitter::targetTransform(const QRectF &target,
                                                  const QRect &viewport)
{
    qreal x_scale = target.width() / viewport.width();
    qreal y_scale = target.height() / viewport.height();

    const QPointF relative_to_viewport = target.topLeft() - viewport.topLeft();
    qreal x_translate = x_scale - 1 + ((relative_to_viewport.x() / viewport.width()) * 2);
    qreal y_translate = -y_scale + 1 - ((relative_to_viewport.y() / viewport.height()) * 2);

    QMatrix4x4 matrix;
    matrix(0,3) = x_translate;
    matrix(1,3) = y_translate;

    matrix(0,0) = x_scale;
    matrix(1,1) = y_scale;

    return matrix;
}

QMatrix3x3 QOpenGLTextureBlitter::sourceTransform(const QRectF &subTexture,
                                                  const QSize &textureSize,
                                                  Origin origin)
{
    qreal x_scale = subTexture.width() / textureSize.width();
    qreal y_scale = subTexture.height() / textureSize.height();

    const QPointF topLeft = subTexture.topLeft();
    qreal x_translate = topLeft.x() / textureSize.width();
    qreal y_translate = topLeft.y() / textureSize.height();

    if (origin == OriginTopLeft) {
        y_scale = -y_scale;
        y_translate = 1 - y_translate;
    }

    QMatrix3x3 matrix;
    matrix(0,2) = x_translate;
    matrix(1,2) = y_translate;

    matrix(0,0) = x_scale;
    matrix(1,1) = y_scale;

    return matrix;
}

QT_END_NAMESPACE
