/*
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ocio_display_filter_vfx2021.h"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_3_2_Core>

#include <cmath>
#include <cstring>

#include <kis_config.h>
#include <kis_debug.h>
#include <opengl/kis_opengl.h>

#if defined(QT_OPENGL_ES_2)
#define GL_RGBA32F_ARB GL_RGBA32F_EXT
#define GL_RGB32F_ARB GL_RGB32F_EXT
#endif

#include "kis_context_thread_locale.h"

OcioDisplayFilter::OcioDisplayFilter(KisExposureGammaCorrectionInterface *interface, QObject *parent)
    : KisDisplayFilter(parent)
    , inputColorSpaceName(0)
    , displayDevice(0)
    , view(0)
    , look(0)
    , swizzle(RGBA)
    , m_interface(interface)
    , m_lut3dTexIDs()
    , m_lut3dUniforms()
    , m_shaderDirty(true)
{
}

OcioDisplayFilter::~OcioDisplayFilter()
{
}

KisExposureGammaCorrectionInterface *OcioDisplayFilter::correctionInterface() const
{
    return m_interface;
}

void OcioDisplayFilter::filter(quint8 *pixels, quint32 numPixels)
{
    // processes that data _in_ place
    if (m_processor) {
        OCIO::PackedImageDesc img(reinterpret_cast<float *>(pixels), numPixels, 1, 4);
        m_processor->getDefaultCPUProcessor()->apply(img);
    }
}

void OcioDisplayFilter::approximateInverseTransformation(quint8 *pixels, quint32 numPixels)
{
    // processes that data _in_ place
    if (m_revereseApproximationProcessor) {
        OCIO::PackedImageDesc img(reinterpret_cast<float *>(pixels), numPixels, 1, 4);
        m_revereseApproximationProcessor->getDefaultCPUProcessor()->apply(img);
    }
}

void OcioDisplayFilter::approximateForwardTransformation(quint8 *pixels, quint32 numPixels)
{
    // processes that data _in_ place
    if (m_forwardApproximationProcessor) {
        OCIO::PackedImageDesc img(reinterpret_cast<float *>(pixels), numPixels, 1, 4);
        m_forwardApproximationProcessor->getDefaultCPUProcessor()->apply(img);
    }
}

bool OcioDisplayFilter::useInternalColorManagement() const
{
    return forceInternalColorManagement;
}

bool OcioDisplayFilter::lockCurrentColorVisualRepresentation() const
{
    return m_lockCurrentColorVisualRepresentation;
}

void OcioDisplayFilter::setLockCurrentColorVisualRepresentation(bool value)
{
    m_lockCurrentColorVisualRepresentation = value;
}

QString OcioDisplayFilter::program() const
{
    return m_program;
}

void OcioDisplayFilter::updateProcessor()
{
    if (!config) {
        return;
    }

    if (!displayDevice) {
        displayDevice = config->getDefaultDisplay();
    }

    if (!view) {
        view = config->getDefaultView(displayDevice);
    }

    if (!inputColorSpaceName) {
        inputColorSpaceName = config->getColorSpaceNameByIndex(0);
    }
    if (!look) {
        look = config->getLookNameByIndex(0);
    }

    if (!displayDevice || !view || !inputColorSpaceName) {
        return;
    }

    OCIO::DisplayViewTransformRcPtr transform = OCIO::DisplayViewTransform::Create();
    transform->setSrc(inputColorSpaceName);
    transform->setDisplay(displayDevice);
    transform->setView(view);

    OCIO::LegacyViewingPipelineRcPtr vpt = OCIO::LegacyViewingPipeline::Create();

    vpt->setDisplayViewTransform(transform);

    /**
     * Look support:
     * As the OCIO docs will tell you, looks are a aesthetic transform that is
     * added onto the mix.
     * A view+display can have it's own assigned Look, or list of looks, and these
     * can be overridden optionally.
     * What the OCIO docs won't tell you is that a display transform won't use the
     * looks attached to it unless "skipColorSpaceConversions" is false...
     * I have no idea what "skipColorSpaceConversions" is beyond what it says on the
     * tin. It is not mentioned in the documentation anywhere. Or on the website.
     * Or how to set it. Or unset it. Why it is apparently set true to begin with.
     * Only that, apparently, this was done with non-color data in mind...
     *
     * Until there's clear documentation on how to use this feature, I am afraid the
     * override is all we can offer.
     */
    if (config->getLook(look)) {
        vpt->setLooksOverride(look);
        vpt->setLooksOverrideEnabled(true);
    }

    OCIO::GroupTransformRcPtr approximateTransform = OCIO::GroupTransform::Create();

    // fstop exposure control -- not sure how that translates to our exposure
    {
        const double exposureGain = pow(2.0, exposure);

        const double minRange = 0.001;
        if (qAbs(blackPoint - whitePoint) < minRange) {
            whitePoint = blackPoint + minRange;
        }

        const double oldMin[] = {blackPoint, blackPoint, blackPoint, 0.0};
        const double oldMax[] = {whitePoint, whitePoint, whitePoint, 1.0};

        const double newMin[] = {0.0, 0.0, 0.0, 0.0};
        const double newMax[] = {exposureGain, exposureGain, exposureGain, 1.0};

        double m44[16];
        double offset4[4];
        OCIO::MatrixTransform::Fit(m44, offset4, oldMin, oldMax, newMin, newMax);
        OCIO::MatrixTransformRcPtr mtx = OCIO::MatrixTransform::Create();
        mtx->setMatrix(m44);
        mtx->setOffset(offset4);
        vpt->setLinearCC(mtx);

        // approximation (no color correction);
        approximateTransform->appendTransform(mtx);
    }

    // channel swizzle
    {
        int channelHot[4];
        switch (swizzle) {
        case LUMINANCE:
            channelHot[0] = 1;
            channelHot[1] = 1;
            channelHot[2] = 1;
            channelHot[3] = 0;
            break;
        case RGBA:
            channelHot[0] = 1;
            channelHot[1] = 1;
            channelHot[2] = 1;
            channelHot[3] = 1;
            break;
        case R:
            channelHot[0] = 1;
            channelHot[1] = 0;
            channelHot[2] = 0;
            channelHot[3] = 0;
            break;
        case G:
            channelHot[0] = 0;
            channelHot[1] = 1;
            channelHot[2] = 0;
            channelHot[3] = 0;
            break;
        case B:
            channelHot[0] = 0;
            channelHot[1] = 0;
            channelHot[2] = 1;
            channelHot[3] = 0;
            break;
        case A:
            channelHot[0] = 0;
            channelHot[1] = 0;
            channelHot[2] = 0;
            channelHot[3] = 1;
        default:;
        }
        double lumacoef[3];
        config->getDefaultLumaCoefs(lumacoef);
        double m44[16];
        double offset[4];
        OCIO::MatrixTransform::View(m44, offset, channelHot, lumacoef);
        OCIO::MatrixTransformRcPtr swizzleTransform = OCIO::MatrixTransform::Create();
        swizzleTransform->setMatrix(m44);
        swizzleTransform->setOffset(offset);
        vpt->setChannelView(swizzleTransform);
    }

    // Post-display transform gamma
    {
        double exponent = 1.0 / std::max(1e-6, gamma);
        const double exponent4f[] = {exponent, exponent, exponent, exponent};
        OCIO::ExponentTransformRcPtr expTransform = OCIO::ExponentTransform::Create();
        expTransform->setValue(exponent4f);
        vpt->setDisplayCC(expTransform);

        // approximation (no color correction);
        approximateTransform->appendTransform(expTransform);
    }

    try {
        AutoSetAndRestoreThreadLocale l;
        m_processor = vpt->getProcessor(config, config->getCurrentContext());
    } catch (OCIO::Exception &e) {
        // XXX: How to not break the OCIO shader now?
        errKrita << "OCIO exception while parsing the current context:" << e.what();
        m_shaderDirty = false;
        return;
    }

    m_forwardApproximationProcessor = config->getProcessor(approximateTransform, OCIO::TRANSFORM_DIR_FORWARD);

    try {
        m_revereseApproximationProcessor = config->getProcessor(approximateTransform, OCIO::TRANSFORM_DIR_INVERSE);
    } catch (...) {
        warnKrita << "OCIO inverted matrix does not exist!";
        // m_revereseApproximationProcessor;
    }

    m_shaderDirty = true;
}

bool OcioDisplayFilter::updateShader()
{
    if (KisOpenGL::hasOpenGLES()) {
        QOpenGLContext *ctx = QOpenGLContext::currentContext();

        KIS_ASSERT_RECOVER_RETURN_VALUE(ctx, false);
        if (ctx->hasExtension("GL_EXT_texture_storage")) {
            if (ctx->hasExtension("GL_OES_texture_float") && ctx->hasExtension("GL_OES_texture_float_linear")) {
                QOpenGLExtraFunctions *f = ctx->extraFunctions();
                if (f) {
                    return updateShaderImpl(f);
                }
            } else {
                dbgKrita << "OcioDisplayFilter::updateShader"
                         << "OpenGL ES v2+ support detected but no OES_texture_float"
                            " or GL_EXT_color_buffer_float were found";
                return false;
            }
        } else {
            dbgKrita << "OcioDisplayFilter::updateShader"
                     << "OpenGL ES v2 support detected but GL_EXT_texture_storage was not found";
            return false;
        }
#if defined(QT_OPENGL_3)
    } else if (KisOpenGL::hasOpenGL3()) {
        QOpenGLFunctions_3_2_Core *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
        if (f) {
            return updateShaderImpl(f);
        }
#endif
    }

    // XXX This option can be removed once we move to Qt 5.7+
    if (KisOpenGL::supportsLoD()) {
#if defined(QT_OPENGL_3)
#if defined(Q_OS_MAC) && defined(QT_OPENGL_3_2)
        QOpenGLFunctions_3_2_Core *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
#else
        QOpenGLFunctions_3_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_0>();
#endif
        if (f) {
            return updateShaderImpl(f);
        }
#endif
    }
#if !defined(QT_OPENGL_ES_2)
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    if (f) {
        return updateShaderImpl(f);
    }
#endif

    return false;
}

template<class F>
bool OcioDisplayFilter::updateShaderImpl(F *f)
{
    // check whether we are allowed to use shaders -- though that should
    // work for everyone these days
    KisConfig cfg(true);
    if (!cfg.useOpenGL())
        return false;

    if (!m_shaderDirty)
        return false;

    if (!f) {
        qWarning() << "Failed to get valid OpenGL functions for OcioDisplayFilter!";
        return false;
    }

    f->initializeOpenGLFunctions();

    bool shouldRecompileShader = false;

    // Step 1: Create a GPU Shader Description
    OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();

#if OCIO_VERSION_HEX >= 0x2010100 || OCIO_VERSION_HEX >= 0x2020000
    if (KisOpenGL::supportsLoD()) {
        shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_ES_3_0);
    } else {
        shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_ES_1_0);
    }
#else
    if (KisOpenGL::supportsLoD()) {
        shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);
    } else {
        shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_2);
    }
#endif

    shaderDesc->setFunctionName("OCIODisplay");
    shaderDesc->setResourcePrefix("ocio_");

    // Step 2: Compute the 3D LUT
#if OCIO_VERSION_HEX >= 0x2010100 || OCIO_VERSION_HEX >= 0x2020000
    // ensure the new GPU pipeline is used with our GLES3 patch
    // this way users won't run into errors when using Angle along with OCIO
    const auto gpu = m_processor->getOptimizedGPUProcessor(OCIO::OptimizationFlags::OPTIMIZATION_DEFAULT);
#else
    const int lut3DEdgeSize = cfg.ocioLutEdgeSize();
    const auto gpu =
        m_processor->getOptimizedLegacyGPUProcessor(OCIO::OptimizationFlags::OPTIMIZATION_DEFAULT, lut3DEdgeSize);
#endif

    gpu->extractGpuShaderInfo(shaderDesc);

    // OCIO v2 assumes you'll use the OglApp helpers
    // these are unusable from a Qt backend, because they rely on GLUT/GLFW
    // ociodisplay original pipeline:
    // https://github.com/AcademySoftwareFoundation/OpenColorIO/blob/508b3f4a0618435aeed2f45058208bdfa99e0887/src/apps/ociodisplay/main.cpp
    // ociodisplay new pipeline is a single call:
    // https://github.com/AcademySoftwareFoundation/OpenColorIO/blob/ffddc3341f5775c7866fe2c93275e1d5e0b0540f/src/apps/ociodisplay/main.cpp#L427
    // we need to replicate this loop:
    // https://github.com/AcademySoftwareFoundation/OpenColorIO/blob/dd59baf555656e09f52c3838e85ccf154497ec1d/src/libutils/oglapphelpers/oglapp.cpp#L191-L223
    // calls functions from here:
    // https://github.com/AcademySoftwareFoundation/OpenColorIO/blob/dd59baf555656e09f52c3838e85ccf154497ec1d/src/libutils/oglapphelpers/glsl.cpp

    for (const auto &tex : m_lut3dTexIDs) {
        f->glDeleteTextures(1, &tex.m_uid);
    }

    m_lut3dTexIDs.clear();

    // This is the first available index for the textures.
    unsigned currIndex = 1;

    // Process the 3D LUT first.

    const unsigned maxTexture3D = shaderDesc->getNum3DTextures();
    for (unsigned idx = 0; idx < maxTexture3D; ++idx) {
        // 1. Get the information of the 3D LUT.

        const char *textureName = nullptr;
        const char *samplerName = nullptr;
        unsigned edgelen = 0;
        OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
        shaderDesc->get3DTexture(idx, textureName, samplerName, edgelen, interpolation);

        if (!textureName || !*textureName || !samplerName || !*samplerName || edgelen == 0) {
            errOpenGL << "The texture data is corrupted";
            return false;
        }

        const float *values = nullptr;
        shaderDesc->get3DTextureValues(idx, values);
        if (!values) {
            errOpenGL << "The texture values are missing";
            return false;
        }

        // 2. Allocate the 3D LUT.

        unsigned texId = 0;
        {
            if (values == nullptr) {
                errOpenGL << "3D LUT" << idx << "Missing texture data";
                return false;
            }

            f->glGenTextures(1, &texId);

            f->glActiveTexture(GL_TEXTURE0 + currIndex);

            f->glBindTexture(GL_TEXTURE_3D, texId);

            {
                if (interpolation == OCIO::INTERP_NEAREST) {
                    f->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    f->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                } else {
                    f->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    f->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }

                f->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                f->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                f->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }

            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F_ARB, edgelen, edgelen, edgelen, 0, GL_RGB, GL_FLOAT, values);
        }

        // 3. Keep the texture id & name for the later enabling.

        m_lut3dTexIDs.push_back({texId, textureName, samplerName, GL_TEXTURE_3D});

        currIndex++;
    }

    // Process the 1D LUTs.

    const unsigned maxTexture2D = shaderDesc->getNumTextures();
    for (unsigned idx = 0; idx < maxTexture2D; ++idx) {
        // 1. Get the information of the 1D LUT.

        const char *textureName = nullptr;
        const char *samplerName = nullptr;
        unsigned width = 0;
        unsigned height = 0;
        OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
        OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
        shaderDesc->getTexture(idx, textureName, samplerName, width, height, channel, interpolation);

        if (!textureName || !*textureName || !samplerName || !*samplerName || width == 0) {
            errOpenGL << "The texture data is corrupted";
            return false;
        }

        const float *values = nullptr;
        shaderDesc->getTextureValues(idx, values);
        if (!values) {
            errOpenGL << "The texture values are missing";
            return false;
        }

        // 2. Allocate the 1D LUT (a 2D texture is needed to hold large LUTs).

        unsigned texId = 0;
        {
            if (values == nullptr) {
                errOpenGL << "1D LUT" << idx << "Missing texture data.";
                return false;
            }

            unsigned internalformat = GL_RGB32F_ARB;
            unsigned format = GL_RGB;

            if (channel == OCIO::GpuShaderCreator::TEXTURE_RED_CHANNEL) {
                internalformat = GL_R32F;
                format = GL_RED;
            }

            f->glGenTextures(1, &texId);

            f->glActiveTexture(GL_TEXTURE0 + currIndex);

#if OCIO_VERSION_HEX >= 0x2010100 || OCIO_VERSION_HEX >= 0x2020000
#else
            // 1D Textures are unsupported by OpenGL ES.
            // https://github.com/AcademySoftwareFoundation/OpenColorIO/issues/1486
            if (height > 1) {
#endif
                f->glBindTexture(GL_TEXTURE_2D, texId);

                {
                    if (interpolation == OCIO::INTERP_NEAREST) {
                        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    } else {
                        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }

                    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                }

                f->glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_FLOAT, values);
#if OCIO_VERSION_HEX >= 0x2010100 || OCIO_VERSION_HEX >= 0x2020000
#else
            } else {
                errOpenGL << "1D texture detected @" << idx << ", not supported by OpenGLES";
                return false;
            }
#endif
        }

        // 3. Keep the texture id & name for the later enabling.

        unsigned type = GL_TEXTURE_2D;
        m_lut3dTexIDs.push_back({texId, textureName, samplerName, type});
        currIndex++;
    }

    // Step 3: Generate the shader text
    QString shaderCacheID = QString::fromLatin1(shaderDesc->getCacheID());
    if (m_program.isEmpty() || shaderCacheID != m_shadercacheid) {
        // dbgKrita << "Computing Shader " << m_shadercacheid;

        m_shadercacheid = shaderCacheID;

        m_program = QString::fromLatin1("%1\n").arg(shaderDesc->getShaderText());
        shouldRecompileShader = true;
    }

    // Step 4: mirror and bind uniforms
    m_lut3dUniforms.clear();

    const unsigned maxUniforms = shaderDesc->getNumUniforms();
    for (unsigned idx = 0; idx < maxUniforms; ++idx) {
        OCIO::GpuShaderDesc::UniformData data;
        const char *name = shaderDesc->getUniform(idx, data);
        if (data.m_type == OCIO::UNIFORM_UNKNOWN) {
            errOpenGL << "Uniform" << idx << "has an unknown type";
            return false;
        }
        // Transfer uniform.
        m_lut3dUniforms.push_back({name, data});
    }

    m_shaderDirty = false;
    return shouldRecompileShader;
}

void OcioDisplayFilter::setupTextures(QOpenGLFunctions *f, QOpenGLShaderProgram *program) const
{
    for (unsigned int idx = 0; idx < m_lut3dTexIDs.size(); ++idx) {
        const auto &data = m_lut3dTexIDs[idx];
        f->glActiveTexture(GL_TEXTURE0 + 1 + idx);
        f->glBindTexture(data.m_type, data.m_uid);
        program->setUniformValue(program->uniformLocation(data.m_samplerName), GLint(1 + idx));
    }

    for (const KisTextureUniform &uniform : m_lut3dUniforms) {
        const int m_handle = program->uniformLocation(uniform.m_name);

        const OCIO::GpuShaderDesc::UniformData &m_data = uniform.m_data;

        // Update value.
        if (m_data.m_getDouble) {
            program->setUniformValue(m_handle, static_cast<const GLfloat>(m_data.m_getDouble()));
        } else if (m_data.m_getBool) {
            program->setUniformValue(m_handle, static_cast<const GLfloat>(m_data.m_getBool() ? 1.0f : 0.0f));
        } else if (m_data.m_getFloat3) {
            program->setUniformValue(m_handle,
                                     m_data.m_getFloat3()[0],
                                     m_data.m_getFloat3()[1],
                                     m_data.m_getFloat3()[2]);
        } else if (m_data.m_vectorFloat.m_getSize && m_data.m_vectorFloat.m_getVector) {
            program->setUniformValueArray(m_handle,
                                          m_data.m_vectorFloat.m_getVector(),
                                          m_data.m_vectorFloat.m_getSize(),
                                          1);
        } else if (m_data.m_vectorInt.m_getSize && m_data.m_vectorInt.m_getVector) {
            program->setUniformValueArray(m_handle, m_data.m_vectorInt.m_getVector(), m_data.m_vectorInt.m_getSize());
        } else {
            errOpenGL << "Uniform" << uniform.m_name << "is not linked to any value";
            continue;
        }
    }
}
