/*
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef OCIO_DISPLAY_FILTER_H
#define OCIO_DISPLAY_FILTER_H

#include <QOpenGLShaderProgram>

#include <OpenColorIO.h>
#include <OpenColorTransforms.h>
#include <OpenColorTypes.h>

#include <kis_display_filter.h>
#include <kis_exposure_gamma_correction_interface.h>

namespace OCIO = OCIO_NAMESPACE;

enum OCIO_CHANNEL_SWIZZLE {
    LUMINANCE,
    RGBA,
    R,
    G,
    B,
    A
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
    template <class F>
    bool updateShaderImpl(F *f);

    void setupTextures(GLFunctions *f, QOpenGLShaderProgram *program) const override;

    KisExposureGammaCorrectionInterface *correctionInterface() const override;

    virtual QString program() const override;

    void updateProcessor();

    OCIO::ConstConfigRcPtr config;

    const char *inputColorSpaceName {nullptr};
    const char *displayDevice {nullptr};
    const char *view {nullptr};
    const char *look {nullptr};
    OCIO_CHANNEL_SWIZZLE swizzle {RGBA};
    float exposure {0.0};
    float gamma {0.0};
    float blackPoint {0.0};
    float whitePoint {0.0};
    bool forceInternalColorManagement {false};

private:

    OCIO::ConstProcessorRcPtr m_processor;
    OCIO::ConstProcessorRcPtr m_revereseApproximationProcessor;
    OCIO::ConstProcessorRcPtr m_forwardApproximationProcessor;

    KisExposureGammaCorrectionInterface *m_interface {nullptr};

    bool m_lockCurrentColorVisualRepresentation {false};

    QString m_program;
    GLuint m_lut3dTexID {0};
    QVector<float> m_lut3d;
    QString m_lut3dcacheid;
    QString m_shadercacheid;

    bool m_shaderDirty {true};
};

#endif // OCIO_DISPLAY_FILTER_H
