/*
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DISPLAY_FILTER_H
#define KIS_DISPLAY_FILTER_H

#include <QObject>

#include <qopengl.h>

#include <kritaui_export.h>

struct KisExposureGammaCorrectionInterface;

#ifndef Q_MOC_RUN
#ifndef Q_OS_MACOS
#define GLFunctions QOpenGLFunctions
#else
#define GLFunctions QOpenGLFunctions_3_2_Core
#endif
#endif
class GLFunctions;
class QOpenGLShaderProgram;

/**
 * @brief The KisDisplayFilter class is the base class for filters that
 * are applied by the canvas to the projection before displaying.
 */
class KRITAUI_EXPORT KisDisplayFilter : public QObject
{
    Q_OBJECT
public:
    explicit KisDisplayFilter(QObject *parent = 0);

    virtual QString program() const = 0;
    virtual void setupTextures(GLFunctions *f, QOpenGLShaderProgram *program) const = 0;
    virtual void filter(quint8 *pixels, quint32 numPixels) = 0;
    virtual void approximateInverseTransformation(quint8 *pixels, quint32 numPixels) = 0;
    virtual void approximateForwardTransformation(quint8 *pixels, quint32 numPixels) = 0;
    virtual bool useInternalColorManagement() const = 0;
    virtual KisExposureGammaCorrectionInterface *correctionInterface() const = 0;
    virtual bool lockCurrentColorVisualRepresentation() const = 0;
    /**
     * @return true if the shader should be recompiled
     */
    virtual bool updateShader() = 0;
};


#endif
