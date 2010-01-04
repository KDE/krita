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

#include "opengl/kis_opengl_gradient_program.h"

#include <QGLWidget>

#include <cfloat>

#include <KoAbstractGradient.h>

#include "opengl/kis_opengl_fragment_shader.h"
#include "kis_debug.h"
#include <kis_paint_device.h>

//-----------------------------------------------------------------------------

static const GLint NUM_COLOR_TEXTURES = 1;
static const int GRADIENT_COLORS_TEXTURE_WIDTH = 256;

//-----------------------------------------------------------------------------

class KisOpenGLGradientShader
{
public:
    virtual ~KisOpenGLGradientShader();

    KisOpenGLShader &shader();
    virtual void setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd) = 0;

protected:
    KisOpenGLGradientShader(KisOpenGLProgram *program, const QString &sourceFilename);

    KisOpenGLProgram *m_program;
    KisOpenGLFragmentShader *m_shader;
};

KisOpenGLGradientShader::KisOpenGLGradientShader(KisOpenGLProgram *program, const QString &sourceFilename)
        : m_program(program),
        m_shader(KisOpenGLFragmentShader::createFromSourceCodeFile(sourceFilename))
{
}

KisOpenGLGradientShader::~KisOpenGLGradientShader()
{
    delete m_shader;
}

KisOpenGLShader &KisOpenGLGradientShader::shader()
{
    Q_ASSERT(m_shader);
    return *m_shader;
}

//-----------------------------------------------------------------------------

class KisOpenGLLinearGradientShader : public KisOpenGLGradientShader
{
public:
    KisOpenGLLinearGradientShader(KisOpenGLProgram *program);

    virtual void setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd);

protected:
    KisOpenGLLinearGradientShader(KisOpenGLProgram *program, const QString &shaderSourceFilename);
};

KisOpenGLLinearGradientShader::KisOpenGLLinearGradientShader(KisOpenGLProgram *program)
        : KisOpenGLGradientShader(program, "linear_gradient.frag")
{
}

KisOpenGLLinearGradientShader::KisOpenGLLinearGradientShader(KisOpenGLProgram *program, const QString &shaderSourceFilename)
        : KisOpenGLGradientShader(program, shaderSourceFilename)
{
}

void KisOpenGLLinearGradientShader::setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd)
{
    Q_ASSERT(m_program);
    if (m_program) {
        m_program->setUniformVariable("gradientVectorStart", gradientVectorStart);

        QPointF gradientVector = gradientVectorEnd - gradientVectorStart;

        m_program->setUniformVariable("normalisedGradientVector", gradientVector);
    }
}

//-----------------------------------------------------------------------------

class KisOpenGLBilinearGradientShader : public KisOpenGLLinearGradientShader
{
public:
    KisOpenGLBilinearGradientShader(KisOpenGLProgram *program);
};

KisOpenGLBilinearGradientShader::KisOpenGLBilinearGradientShader(KisOpenGLProgram *program)
        : KisOpenGLLinearGradientShader(program, "bilinear_gradient.frag")
{
}

//-----------------------------------------------------------------------------

class KisOpenGLRadialGradientShader : public KisOpenGLGradientShader
{
public:
    KisOpenGLRadialGradientShader(KisOpenGLProgram *program);

    virtual void setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd);
};

KisOpenGLRadialGradientShader::KisOpenGLRadialGradientShader(KisOpenGLProgram *program)
        : KisOpenGLGradientShader(program, "radial_gradient.frag")
{
}

void KisOpenGLRadialGradientShader::setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd)
{
    Q_UNUSED(gradientVectorEnd);

    Q_ASSERT(m_program);
    if (m_program) {
        m_program->setUniformVariable("gradientVectorStart", gradientVectorStart);
    }
}

//-----------------------------------------------------------------------------

class KisOpenGLSquareGradientShader : public KisOpenGLGradientShader
{
public:
    KisOpenGLSquareGradientShader(KisOpenGLProgram *program);

    virtual void setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd);
};

KisOpenGLSquareGradientShader::KisOpenGLSquareGradientShader(KisOpenGLProgram *program)
        : KisOpenGLGradientShader(program, "square_gradient.frag")
{
}

void KisOpenGLSquareGradientShader::setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd)
{
    Q_ASSERT(m_program);
    if (m_program) {
        m_program->setUniformVariable("gradientVectorStart", gradientVectorStart);

        QPointF gradientVector = gradientVectorEnd - gradientVectorStart;

        m_program->setUniformVariable("normalisedGradientVector", gradientVector);
    }
}

//-----------------------------------------------------------------------------

class KisOpenGLConicalGradientShader : public KisOpenGLGradientShader
{
public:
    KisOpenGLConicalGradientShader(KisOpenGLProgram *program);

    virtual void setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd);

protected:
    KisOpenGLConicalGradientShader(KisOpenGLProgram *program, const QString &shaderSourceFilename);
};

KisOpenGLConicalGradientShader::KisOpenGLConicalGradientShader(KisOpenGLProgram *program)
        : KisOpenGLGradientShader(program, "conical_gradient.frag")
{
}

KisOpenGLConicalGradientShader::KisOpenGLConicalGradientShader(KisOpenGLProgram *program, const QString &shaderSourceFilename)
        : KisOpenGLGradientShader(program, shaderSourceFilename)
{
}

void KisOpenGLConicalGradientShader::setGradientVector(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd)
{
    Q_ASSERT(m_program);
    if (m_program) {
        m_program->setUniformVariable("gradientVectorStart", gradientVectorStart);

        QPointF gradientVector = gradientVectorEnd - gradientVectorStart;

        // Get angle from 0 to 2 PI.
        GLfloat gradientVectorAngle = atan2(gradientVector.y(), gradientVector.x()) + M_PI;

        m_program->setUniformVariable("gradientVectorAngle", gradientVectorAngle);
    }
}

//-----------------------------------------------------------------------------

class KisOpenGLConicalSymetricGradientShader : public KisOpenGLConicalGradientShader
{
public:
    KisOpenGLConicalSymetricGradientShader(KisOpenGLProgram *program);
};

KisOpenGLConicalSymetricGradientShader::KisOpenGLConicalSymetricGradientShader(KisOpenGLProgram *program)
        : KisOpenGLConicalGradientShader(program, "conical_symetric_gradient.frag")
{
}

//-----------------------------------------------------------------------------

KisOpenGLGradientProgram::KisOpenGLGradientProgram(const KoAbstractGradient *gradient,
        KisGradientPainter::enumGradientShape shape,
        KisGradientPainter::enumGradientRepeat repeat,
        bool reverseGradient,
        const KoColorSpace *colorSpace,
        KoColorProfile *monitorProfile,
        double opacity)
{
    createGradientColorsTexture(gradient, repeat, reverseGradient, colorSpace, monitorProfile, opacity);

    switch (shape) {
    case KisGradientPainter::GradientShapeLinear:
        m_gradientShader = new KisOpenGLLinearGradientShader(this);
        break;
    case KisGradientPainter::GradientShapeBiLinear:
        m_gradientShader = new KisOpenGLBilinearGradientShader(this);
        break;
    case KisGradientPainter::GradientShapeRadial:
        m_gradientShader = new KisOpenGLRadialGradientShader(this);
        break;
    case KisGradientPainter::GradientShapeSquare:
        m_gradientShader = new KisOpenGLSquareGradientShader(this);
        break;
    case KisGradientPainter::GradientShapeConical:
        m_gradientShader = new KisOpenGLConicalGradientShader(this);
        break;
    case KisGradientPainter::GradientShapeConicalSymetric:
        m_gradientShader = new KisOpenGLConicalSymetricGradientShader(this);
        break;
    }
    Q_CHECK_PTR(m_gradientShader);

    attachShader(m_gradientShader->shader());
    link();
    dbgTools << getInfoLog();
}

KisOpenGLGradientProgram::~KisOpenGLGradientProgram()
{
    KisOpenGL::makeContextCurrent();
    glDeleteTextures(NUM_COLOR_TEXTURES, &m_gradientColorsTexture);
    delete m_gradientShader;
}

void KisOpenGLGradientProgram::createGradientColorsTexture(const KoAbstractGradient *gradient,
        KisGradientPainter::enumGradientRepeat repeat,
        bool reverseGradient,
        const KoColorSpace *colorSpace,
        KoColorProfile *monitorProfile,
        double opacity)
{
    KisOpenGL::makeContextCurrent();

    glGenTextures(NUM_COLOR_TEXTURES, &m_gradientColorsTexture);

    glBindTexture(GL_TEXTURE_1D, m_gradientColorsTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint textureWrap;

    switch (repeat) {
    default:
    case KisGradientPainter::GradientRepeatNone:
        textureWrap = GL_CLAMP_TO_EDGE;
        break;
    case KisGradientPainter::GradientRepeatForwards:
        textureWrap = GL_REPEAT;
        break;
    case KisGradientPainter::GradientRepeatAlternate:
        textureWrap = GL_MIRRORED_REPEAT;
        break;
    }

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, textureWrap);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    QRgb gradientColorsArray[GRADIENT_COLORS_TEXTURE_WIDTH];

    for (int texelIndex = 0; texelIndex < GRADIENT_COLORS_TEXTURE_WIDTH; ++texelIndex) {

        double t = (double)texelIndex / (GRADIENT_COLORS_TEXTURE_WIDTH - 1);

        if (reverseGradient) {
            t = 1 - t;
        }

        KoColor texelColor(colorSpace);
        QColor texelRGB;

        gradient->colorAt(texelColor, t);
        texelColor.colorSpace()->toQColor(texelColor.data(), &texelRGB, monitorProfile);
        gradientColorsArray[texelIndex] = qRgba(texelRGB.red(),
                                                texelRGB.green(),
                                                texelRGB.blue(),
                                                qRound(texelRGB.alpha() * opacity));
    }

    KIS_OPENGL_CLEAR_ERROR();
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, GRADIENT_COLORS_TEXTURE_WIDTH, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, gradientColorsArray);
    KIS_OPENGL_PRINT_ERROR();
}

void KisOpenGLGradientProgram::activate(const QPointF &gradientVectorStart, const QPointF &gradientVectorEnd)
{
    KisOpenGLProgram::activate();

    Q_ASSERT(m_gradientShader);
    if (m_gradientShader) {
        m_gradientShader->setGradientVector(gradientVectorStart, gradientVectorEnd);
        setUniformVariable("gradientColors", (GLint) 0);
        glBindTexture(GL_TEXTURE_1D, m_gradientColorsTexture);
    }
}

