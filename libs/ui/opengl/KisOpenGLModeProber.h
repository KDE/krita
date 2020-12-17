/*
 *  SPDX-FileCopyrightText: 2017 Alvin Wong <alvinhochun@gmail.com>
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISOPENGLMODEPROBER_H
#define KISOPENGLMODEPROBER_H

#include "kritaui_export.h"
#include "kis_config.h"
#include <QSurfaceFormat>
#include "KisSurfaceColorSpace.h"
#include <boost/optional.hpp>
#include "kis_opengl.h"

class KoColorProfile;

class KRITAUI_EXPORT KisOpenGLModeProber
{
public:
    class Result;

public:
    KisOpenGLModeProber();
    ~KisOpenGLModeProber();

    static KisOpenGLModeProber* instance();

    bool useHDRMode() const;
    QSurfaceFormat surfaceformatInUse() const;

    const KoColorProfile *rootSurfaceColorProfile() const;

    boost::optional<Result> probeFormat(const KisOpenGL::RendererConfig &rendererConfig,
                                        bool adjustGlobalState = true);

    static bool fuzzyCompareColorSpaces(const KisSurfaceColorSpace &lhs,
                                        const KisSurfaceColorSpace &rhs);
    static QString angleRendererToString(KisOpenGL::AngleRenderer renderer);

public:
    static void initSurfaceFormatFromConfig(KisConfig::RootSurfaceFormat config,
                                            QSurfaceFormat *format);
    static bool isFormatHDR(const QSurfaceFormat &format);
};

class KisOpenGLModeProber::Result {
public:
    Result(QOpenGLContext &context);

    int glMajorVersion() const {
        return m_glMajorVersion;
    }

    int glMinorVersion() const {
        return m_glMinorVersion;
    }

    bool supportsDeprecatedFunctions() const {
        return m_supportsDeprecatedFunctions;
    }

    bool isOpenGLES() const {
        return m_isOpenGLES;
    }

    QString rendererString() const {
        return m_rendererString;
    }

    QString driverVersionString() const {
        return m_driverVersionString;
    }

    bool isSupportedVersion() const {
        return
#ifdef Q_OS_MACOS
                ((m_glMajorVersion * 100 + m_glMinorVersion) >= 302)
#else
                (m_glMajorVersion >= 3 && (m_supportsDeprecatedFunctions || m_isOpenGLES)) ||
                ((m_glMajorVersion * 100 + m_glMinorVersion) == 201)
#endif
                ;
    }

    bool supportsLoD() const {
        return (m_glMajorVersion * 100 + m_glMinorVersion) >= 300;
    }

    bool hasOpenGL3() const {
        return (m_glMajorVersion * 100 + m_glMinorVersion) >= 302;
    }

    bool supportsFenceSync() const {
        return m_glMajorVersion >= 3;
    }

    bool supportsFBO() const {
        return m_supportsFBO;
    }

#ifdef Q_OS_WIN
    // This is only for detecting whether ANGLE is being used.
    // For detecting generic OpenGL ES please check isOpenGLES
    bool isUsingAngle() const {
        return m_rendererString.startsWith("ANGLE", Qt::CaseInsensitive);
    }
#endif

    QString shadingLanguageString() const
    {
        return m_shadingLanguageString;
    }

    QString vendorString() const
    {
        return m_vendorString;
    }

    QSurfaceFormat format() const
    {
        return m_format;
    }

private:
    int m_glMajorVersion = 0;
    int m_glMinorVersion = 0;
    bool m_supportsDeprecatedFunctions = false;
    bool m_isOpenGLES = false;
    bool m_supportsFBO = false;
    QString m_rendererString;
    QString m_driverVersionString;
    QString m_vendorString;
    QString m_shadingLanguageString;
    QSurfaceFormat m_format;
};

#endif // KISOPENGLMODEPROBER_H
