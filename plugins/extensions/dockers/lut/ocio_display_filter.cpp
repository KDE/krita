/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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
#include "ocio_display_filter.h"
#include <math.h>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

#include <kis_config.h>

#ifdef HAVE_OPENGL
#include <opengl/kis_opengl.h>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Core>
#endif


static const int LUT3D_EDGE_SIZE = 32;

OcioDisplayFilter::OcioDisplayFilter(KisExposureGammaCorrectionInterface *interface, QObject *parent)
    : KisDisplayFilter(parent)
    , inputColorSpaceName(0)
    , displayDevice(0)
    , view(0)
    , swizzle(RGBA)
    , m_interface(interface)
    #ifdef HAVE_OPENGL
    , m_lut3dTexID(0)
    #endif
{
}

OcioDisplayFilter::~OcioDisplayFilter()
{
}

KisExposureGammaCorrectionInterface* OcioDisplayFilter::correctionInterface() const
{
    return m_interface;
}

void OcioDisplayFilter::filter(quint8 *pixels, quint32 numPixels)
{
    // processes that data _in_ place
    if (m_processor) {
        OCIO::PackedImageDesc img(reinterpret_cast<float*>(pixels), numPixels, 1, 4);
        m_processor->apply(img);
    }
}

void OcioDisplayFilter::approximateInverseTransformation(quint8 *pixels, quint32 numPixels)
{
    // processes that data _in_ place
    if (m_revereseApproximationProcessor) {
        OCIO::PackedImageDesc img(reinterpret_cast<float*>(pixels), numPixels, 1, 4);
        m_revereseApproximationProcessor->apply(img);
    }
}

void OcioDisplayFilter::approximateForwardTransformation(quint8 *pixels, quint32 numPixels)
{
    // processes that data _in_ place
    if (m_forwardApproximationProcessor) {
        OCIO::PackedImageDesc img(reinterpret_cast<float*>(pixels), numPixels, 1, 4);
        m_forwardApproximationProcessor->apply(img);
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

#ifdef HAVE_OPENGL

QString OcioDisplayFilter::program() const
{
    return m_program;
}

GLuint OcioDisplayFilter::lutTexture() const
{
    return m_lut3dTexID;
}
#endif

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

    OCIO::DisplayTransformRcPtr transform = OCIO::DisplayTransform::Create();
    transform->setInputColorSpaceName(inputColorSpaceName);
    transform->setDisplay(displayDevice);
    transform->setView(view);

    OCIO::GroupTransformRcPtr approximateTransform = OCIO::GroupTransform::Create();

    // fstop exposure control -- not sure how that translates to our exposure
    {
        float exposureGain = powf(2.0f, exposure);

        const qreal minRange = 0.001;
        if (qAbs(blackPoint - whitePoint) < minRange) {
            whitePoint = blackPoint + minRange;
        }

        const float oldMin[] = { blackPoint, blackPoint, blackPoint, 0.0f };
        const float oldMax[] = { whitePoint, whitePoint, whitePoint, 1.0f };

        const float newMin[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        const float newMax[] = { exposureGain, exposureGain, exposureGain, 1.0f };

        float m44[16];
        float offset4[4];
        OCIO::MatrixTransform::Fit(m44, offset4, oldMin, oldMax, newMin, newMax);
        OCIO::MatrixTransformRcPtr mtx =  OCIO::MatrixTransform::Create();
        mtx->setValue(m44, offset4);
        transform->setLinearCC(mtx);

        // approximation (no color correction);
        approximateTransform->push_back(mtx);
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
        default:
            ;
        }
        float lumacoef[3];
        config->getDefaultLumaCoefs(lumacoef);
        float m44[16];
        float offset[4];
        OCIO::MatrixTransform::View(m44, offset, channelHot, lumacoef);
        OCIO::MatrixTransformRcPtr swizzle = OCIO::MatrixTransform::Create();
        swizzle->setValue(m44, offset);
        transform->setChannelView(swizzle);
    }

    // Post-display transform gamma
    {
        float exponent = 1.0f/std::max(1e-6f, static_cast<float>(gamma));
        const float exponent4f[] = { exponent, exponent, exponent, exponent };
        OCIO::ExponentTransformRcPtr expTransform =  OCIO::ExponentTransform::Create();
        expTransform->setValue(exponent4f);
        transform->setDisplayCC(expTransform);

        // approximation (no color correction);
        approximateTransform->push_back(expTransform);
    }

    m_processor = config->getProcessor(transform);

    m_forwardApproximationProcessor = config->getProcessor(approximateTransform, OCIO::TRANSFORM_DIR_FORWARD);

    try {
        m_revereseApproximationProcessor = config->getProcessor(approximateTransform, OCIO::TRANSFORM_DIR_INVERSE);
    } catch (...) {
        warnKrita << "OCIO inverted matrix does not exist!";
        //m_revereseApproximationProcessor;
    }

#ifdef HAVE_OPENGL

    // check whether we are allowed to use shaders -- though that should
    // work for everyone these days
    KisConfig cfg;
    if (!cfg.useOpenGL()) return;

    QOpenGLFunctions_3_2_Core *glFuncs3 = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
    const int lut3DEdgeSize = cfg.ocioLutEdgeSize();

    if (m_lut3d.size() == 0) {
        //dbgKrita << "generating lut";
        glFuncs3->glGenTextures(1, &m_lut3dTexID);

        int num3Dentries = 3 * lut3DEdgeSize * lut3DEdgeSize * lut3DEdgeSize;
        m_lut3d.fill(0.0, num3Dentries);

        glFuncs3->glActiveTexture(GL_TEXTURE1);
        glFuncs3->glBindTexture(GL_TEXTURE_3D, m_lut3dTexID);

        glFuncs3->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glFuncs3->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFuncs3->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glFuncs3->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFuncs3->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glFuncs3->glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB,
                              lut3DEdgeSize, lut3DEdgeSize, lut3DEdgeSize,
                              0, GL_RGB, GL_FLOAT, &m_lut3d.constData()[0]);
    }

    // Step 1: Create a GPU Shader Description
    OCIO::GpuShaderDesc shaderDesc;

    shaderDesc.setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);

    shaderDesc.setFunctionName("OCIODisplay");
    shaderDesc.setLut3DEdgeLen(lut3DEdgeSize);


    // Step 2: Compute the 3D LUT
    QString lut3dCacheID = QString::fromLatin1(m_processor->getGpuLut3DCacheID(shaderDesc));
    if(lut3dCacheID != m_lut3dcacheid)
    {
        //dbgKrita << "Computing 3DLut " << m_lut3dcacheid;
        m_lut3dcacheid = lut3dCacheID;
        m_processor->getGpuLut3D(&m_lut3d[0], shaderDesc);

        glFuncs3->glBindTexture(GL_TEXTURE_3D, m_lut3dTexID);
        glFuncs3->glTexSubImage3D(GL_TEXTURE_3D, 0,
                                 0, 0, 0,
                                 lut3DEdgeSize, lut3DEdgeSize, lut3DEdgeSize,
                                 GL_RGB, GL_FLOAT, &m_lut3d[0]);
    }


    // Step 3: Generate the shader text
    QString shaderCacheID = QString::fromLatin1(m_processor->getGpuShaderTextCacheID(shaderDesc));
    if (m_program.isEmpty() || shaderCacheID != m_shadercacheid) {
        //dbgKrita << "Computing Shader " << m_shadercacheid;

        m_shadercacheid = shaderCacheID;

        std::ostringstream os;
        os << m_processor->getGpuShaderText(shaderDesc) << "\n";

        m_program = QString::fromLatin1(os.str().c_str());
    }


#endif

}
