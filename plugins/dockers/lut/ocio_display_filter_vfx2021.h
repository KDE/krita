/*
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef OCIO_DISPLAY_FILTER_H
#define OCIO_DISPLAY_FILTER_H

#include <vector>

#include <QOpenGLShaderProgram>

#include <OpenColorIO.h>
#include <OpenColorTransforms.h>
#include <OpenColorTypes.h>

#include <kis_display_filter.h>
#include <kis_exposure_gamma_correction_interface.h>

namespace OCIO = OCIO_NAMESPACE;

enum OCIO_CHANNEL_SWIZZLE { LUMINANCE, RGBA, R, G, B, A };

struct KisTextureUniform {
public:
    KisTextureUniform(const QString &name, const OCIO::GpuShaderDesc::UniformData &data)
        : m_name(name)
        , m_data(data)
    {
    }

    QString m_name;
    OCIO::GpuShaderDesc::UniformData m_data;

private:
    KisTextureUniform() = delete;
};

struct KisTextureEntry {
    unsigned m_uid = -1;
    QString m_textureName;
    QString m_samplerName;
    unsigned m_type = -1;

    KisTextureEntry(unsigned uid, const QString &textureName, const QString &samplerName, unsigned type)
        : m_uid(uid)
        , m_textureName(textureName)
        , m_samplerName(samplerName)
        , m_type(type)
    {
    }
};

class OcioDisplayFilter : public KisDisplayFilter
{
    Q_OBJECT
public:
    explicit OcioDisplayFilter(KisExposureGammaCorrectionInterface *interface, QObject *parent = 0);
    ~OcioDisplayFilter();

    void filter(quint8 *pixels, quint32 numPixels) override;
    void approximateInverseTransformation(quint8 *pixels, quint32 numPixels) override;
    void approximateForwardTransformation(quint8 *pixels, quint32 numPixels) override;
    bool useInternalColorManagement() const override;
    bool lockCurrentColorVisualRepresentation() const override;
    void setLockCurrentColorVisualRepresentation(bool value);

    bool updateShader() override;

    template<class F>
    bool updateShaderImpl(F *f);

    void setupTextures(QOpenGLFunctions *f, QOpenGLShaderProgram *program) const override;

    KisExposureGammaCorrectionInterface *correctionInterface() const override;

    QString program() const override;

    void updateProcessor();

    OCIO::ConstConfigRcPtr config;

    const char *inputColorSpaceName;
    const char *displayDevice;
    const char *view;
    const char *look;
    OCIO_CHANNEL_SWIZZLE swizzle;
    double exposure;
    double gamma;
    double blackPoint;
    double whitePoint;
    bool forceInternalColorManagement;

private:
    OCIO::ConstProcessorRcPtr m_processor;
    OCIO::ConstProcessorRcPtr m_revereseApproximationProcessor;
    OCIO::ConstProcessorRcPtr m_forwardApproximationProcessor;

    KisExposureGammaCorrectionInterface *m_interface;

    bool m_lockCurrentColorVisualRepresentation;

    QString m_program;
    std::vector<KisTextureEntry> m_lut3dTexIDs;
    QString m_lut3dcacheid;
    QString m_shadercacheid;
    std::vector<KisTextureUniform> m_lut3dUniforms;

    bool m_shaderDirty;
};

#endif // OCIO_DISPLAY_FILTER_H
